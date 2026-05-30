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
#include <string>
#include <sys/types.h>
#include<sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include <openssl/ssl.h>
#include <openssl/err.h>


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

    SSL_CTX *ctx = nullptr;
    SSL *ssl = nullptr;
    bool is_https = (url.schema == "https");

    if (is_https) {
        const SSL_METHOD *method = TLS_client_method();
        ctx = SSL_CTX_new(method);
        if (!ctx) {
            std::cerr << "Unable to create SSL context" << std::endl;
            close(sockFd);
            return "";
        }
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, sockFd);
        SSL_set_tlsext_host_name(ssl, url.host.c_str());

        if (SSL_connect(ssl) <= 0) {
            std::cerr << "SSL connect failed to " << url.host << std::endl;
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            close(sockFd);
            return "";
        }
    }
        std::string request = "GET " + url.path + " HTTP/1.1\r\n" +
                          "Host: " + url.host + "\r\n" +
                          "Connection: close\r\n" +
                          "User-Agent: CppWebCrawler/1.0\r\n" +
                          "\r\n";

        // if(send(sockFd, request.c_str(), request.length(), 0)==-1){
        //     std::cerr<<"Failed to Send the request\n";
        //     close(sockFd);
        //     return "";
        // }

        

        bool send_failed = false;
        if (is_https) {
            if (SSL_write(ssl, request.c_str(), request.length()) <= 0) {
                send_failed = true;
            }
        } else {
            if (send(sockFd, request.c_str(), request.length(), 0) == -1) {
                send_failed = true;
            }
        }

        if (send_failed) {
            std::cerr << "Failed to send request to " << url.host << std::endl;
            if (is_https) {
                SSL_free(ssl);
                SSL_CTX_free(ctx);
            }
            close(sockFd);
            return "";
        }

        http_parser::HttpParser parser;
        http_parser::HttpResponse resp;
        char buffer[4096];
        ssize_t bytes_received;


        while (true) {
        if (is_https) {
            bytes_received = SSL_read(ssl, buffer, sizeof(buffer));
        } else {
            bytes_received = recv(sockFd, buffer, sizeof(buffer), 0);
        }
        
        if (bytes_received <= 0) break;

        size_t consumed = parser.excute(buffer, bytes_received, resp);
        (void)consumed; // mark unused
        if (parser.isError()) {
            std::cerr << "HTTP Parse error on " << url.toString() << std::endl;
            break;
        }
        if (parser.isComplete()) {
            break;
        }
    }

        if (is_https) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            SSL_CTX_free(ctx);
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
        std::string filename = std::to_string(currDepth) + ".html";
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
                    if(isPage(urlStr)){
                        task.push({resolved,currDepth + 1});
                    }
                }
            }
        }
    }
    bool WebCrawler::isPage(const std::string& link) {
        static const std::vector<std::string> exts = {
            ".css", ".js", ".png", ".jpg", ".jpeg",
            ".gif", ".svg", ".ico", ".pdf",
            ".zip", ".mp4", ".webm",".woff",".woff2"
        };

        for (const auto& ext : exts) {
            if (link.size() >= ext.size() &&
                link.substr(link.size() - ext.size()) == ext)
                return false;
        }
        return true;
    }
}
