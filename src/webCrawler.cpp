/*Seed URL
   ↓
Fetch webpage
   ↓
Parse HTTP response
   ↓
Extract links
   ↓
Resolve relative URLs
   ↓
Add new URLs to queue
   ↓
Repeat*/

#include "../include/http_parser.hpp"
#include <cstddef>
#include <cstring>
#include <iostream>
#include "../include/webCrawler.hpp"
#include <sys/types.h>
#include<sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fstream>
#include <cstring>


namespace crawler {
    WebCrawler::WebCrawler(int maxDepth,bool samedomainOnly):maxDepth(maxDepth) , sameDomainOnly(samedomainOnly){}

    void WebCrawler::start(const std::string& seedurl){
        auto parsed = Url::parse(seedurl);
        if(!parsed){
            std::cerr<<"Invalid Seed Url \n";
            return ;
        }
        seedUrlObj = *parsed;
        task.push({*parsed,0});
        visted.insert(parsed->toString());
        while(!task.empty()){
            auto t = task.front();
            task.pop();
            processUrl(t.url, t.depth);
        }
    }

    std::string WebCrawler::fetch(const Url& url){
        struct addrinfo hints,*res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if(getaddrinfo(url.host.c_str(),  url.port.c_str(), &hints,&res)!=0){
            std::cerr<<"Failed Resove ";
            return "";
        }

        int sockFd = -1;
        struct addrinfo* p;
        for(p=res;p!=nullptr;p = p->ai_next){
            sockFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if(sockFd == -1)continue;
            if(connect(sockFd, p->ai_addr, p->ai_addrlen)==0)break;

            close(sockFd);
            sockFd = -1;
        }

        freeaddrinfo(res);

        if(sockFd == -1){
            std::cerr<<"Failed to Connect "<<url.host<<url.port<<std::endl;
            return "";
        }

        std::string request = "GET " + url.path + " HTTP/1.1\r\n" +
                          "Host: " + url.host + "\r\n" +
                          "Connection: close\r\n" +
                          "User-Agent: CppWebCrawler/1.0\r\n" +
                          "\r\n";

        if(send(sockFd, request.c_str(), request.length(), 0)==-1){
            std::cerr<<"Failed to Send the request\n";
            close(sockFd);
            return "";
        }

        http_parser::HttpParser parser;
        http_parser::HttpResponse resp;
        char buffer[4096];
        size_t byteLen;


        while((byteLen = recv(sockFd, buffer, sizeof(buffer), 0))>0){
            size_t consumed = parser.excute(buffer, byteLen,  resp);
            (void)consumed;

            if(parser.isError()){
                std::cerr<<"Parser Error\n";
                return "";
            }
            if(parser.isComplete())break;
        }
        close(sockFd);
        if(resp.statuscode != 200){
            std::cerr<<"Status Code Error ("<<resp.statusMessage<<" )\n";
            return "";
        }

        std::string body(resp.body.begin(),resp.body.end());

        return body;

    }

    std::vector<std::string> WebCrawler::extractLink(const std::string& html){
        std::vector<std::string> links;
        size_t pos = 0;

        while((pos = html.find("href=\"", pos)) !=std::string::npos){
            pos += 6;
            size_t endPos = html.find("\"",pos);
            if(endPos != std::string::npos){
                std::string t;
                t = html.substr(pos,endPos-pos);
                links.push_back(t);
                pos = endPos +1;
            }else{
                break;
            }
        }
        return links;
    }

    void WebCrawler::processUrl(const Url& url,int currDepth){

        std::cout<<"Crawling at "<<currDepth<<"/"<<maxDepth<<" "<<url.toString()<<std::endl;
        std::string html = fetch(url);
        if(html.empty())return;

        std::string safeName = url.toString();
        for(char &c : safeName) {
            if(c == '/' || c == ':' || c == '?' || c == '&' || c == '=') c = '_';
        }
        std::string filename = "hello.html";
        std::ofstream outFile(filename);
        if(outFile.is_open()){
            outFile << html;
            outFile.close();
            std::cout << "Saved to " << filename << std::endl;
        }

        if(currDepth>=maxDepth)return;

        std::vector<std::string> links = extractLink(html);

        for(const auto& link:links){
            auto resolvedOpt = Url::resolve(url, link);

            if(resolvedOpt){
                Url resolved = *resolvedOpt;

                if(sameDomainOnly && resolved.host != seedUrlObj.host){
                    continue;
                }

                std::string urlStr = resolved.toString();
                if(visted.find(urlStr)==visted.end()){
                    visted.insert(urlStr);
                    task.push({resolved,currDepth + 1});
                }
            }
        }
    }
}
