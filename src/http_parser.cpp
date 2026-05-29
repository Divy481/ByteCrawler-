#include "../include/http_parser.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdio>
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
        std::size_t consumed = 0;

        while(consumed<len && state_ !=State::Error && state_ !=State::Complete){

            char ch = data[consumed++];

            switch (state_) {
                case State::StatusLine:
                case State::Headers:
                case State::ChunkSize:
                case State::Trailer:{
                    if(ch !='\n'){
                        if(!buffer_.empty() && buffer_.back() =='\r'){
                            buffer_.pop_back();
                        }

                        if(state_ == State::StatusLine){
                            if(parserStatusLine(resp)){
                                state_ = State::Headers;
                            }else{
                                state_ = State::Error;
                            }
                        }else if(state_==State::Headers){
                            if(buffer_.empty()){
                                if(resp.headers.find("transfer-encoding") != resp.headers.end() &&
                                    resp.headers["transfer-encoding"].find("chunked") != std::string::npos){
                                        state_ = State::ChunkSize;
                                    }else if(resp.headers.find("content-length") !=resp.headers.end()){
                                        if(contentLength_ ==0){
                                            state_ = State::Complete;
                                        }else{
                                            state_ = State::Body;
                                        }
                                    }else{
                                        state_ = State::Complete;
                                    }
                                }else{
                                    if(!parserHeader(resp))state_ = State::Error;
                                }
                        }else if(state_ == State::ChunkSize){
                            std::size_t semiPos = buffer_.find(';');
                            std::string hexStr = (semiPos!=std::string::npos)? buffer_.substr(0,semiPos) : buffer_;
                            trim(hexStr);

                            if(hexStr.empty()){
                                state_ = State::Error;
                                break;
                            }

                            try{
                                chunkSize_ = std::stoll(hexStr,nullptr,16);
                            }catch(...){
                                state_ = State::Error;
                                break;
                            }

                            if(chunkSize_ == 0){
                                state_ = State::Trailer;
                            }else{
                                chunkByteRead_ = 0;
                                state_ = State::ChunkData;
                            }
                        }else if(state_ ==State::Trailer){
                            if(buffer_.empty())state_ = State::Complete;
                        }
                        buffer_.clear();
                    }else{
                        buffer_.push_back(ch);
                    }
                    break;
                }
                case State::Body:{
                    resp.body.push_back(ch);
                    bodyByteRead_++;
                    if(bodyByteRead_>=contentLength_){
                        state_ = State::Complete;
                    }
                    break;
                }

                case State::ChunkData:{
                    resp.body.push_back(ch);
                    chunkByteRead_++;
                    if(chunkByteRead_>=chunkSize_){
                        state_ = State::ChunkCRLF;
                    }
                    break;
                }

                case State::ChunkCRLF:{
                    if(ch =='\n'){
                        if(!buffer_.empty() && buffer_.back() =='\r'){
                            buffer_.pop_back();
                        }
                        state_ = State::ChunkSize;
                        buffer_.clear();
                    }else if(ch !='\r'){
                        buffer_.push_back(ch);
                    }
                    break;
                }

                case State::Complete:
                case State::Error:
                case State::ChunkExtension:
                     break;
            }
        }
        return consumed;
    }
}
