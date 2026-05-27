#include "../include/http_parser.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>

namespace http_parser {
    HttpParser::HttpParser(){
        reset();
    }

    void HttpParser::reset(){
        state_ = State::StatusLine;
        buffer_.clear();
        contentLength_ = 0;
        chunkSize_ = 0;
        bodyByteRead_ = 0;
        chunkByteRead_ = 0;

    }

    void HttpParser::trim(std::string& str){
        str.erase(str.begin(),std::find_if(str.begin(),str.end(),[](unsigned char ch){
            return !std::isspace(ch);
        }));

        str.erase(std::find_if(str.rbegin(),str.rend(),[](unsigned char ch){
            return !std::isspace(ch);
        }).base(),str.end());
    }

    void HttpParser::toLower(std::string& str){
        std::transform(str.begin(),str.end(),str.begin(),[](unsigned char ch){
            return std::tolower(ch);
        });
    }

    bool HttpParser::parserStatusLine(HttpResponse& resp){
        std::size_t pos1 = buffer_.find(' ');
        if(pos1 == std::string::npos)return false;

        std::string versionString = buffer_.substr(0,pos1);

        if(versionString.find("HTTP/")!=0)return false;

        std::size_t dotPos = versionString.find('.',5);
        if(dotPos == std::string::npos)return false;

        resp.httpMajorVersion = std::stoi(versionString.substr(5,dotPos-5));
        resp.httpMinorVersion = std::stoi(versionString.substr(dotPos+1));

        std::size_t pos2 = buffer_.find(' ',pos1+1);
        if(pos2 == std::string::npos){
            resp.statuscode = std::stoi(buffer_.substr(pos1+1));
            resp.statusMessage = "";
        }else{
            resp.statuscode  = std::stoi(buffer_.substr(pos1+1,pos2-pos1-1));
            resp.statusMessage = buffer_.substr(pos2+1);
            trim(resp.statusMessage);
        }

        return true;
    }

    bool HttpParser::parserHeader(HttpResponse& resp){
        std::size_t pos1 = buffer_.find(':');
        if(pos1 == std::string::npos)return false;

        std::string key = buffer_.substr(0,pos1);
        std::string val = buffer_.substr(pos1+1);

        trim(key);
        toLower(key);
        trim(val);

        resp.headers[key] = val;

        if(key == "content-length"){
            contentLength_ = std::stoll(val);
        }

        return true;
    }

    std::size_t HttpParser::excute(const char* data,size_t len,HttpResponse& resp){
        
    }
}