#pragma once 
#include <map>
#include <vector>
#include <string>

namespace http_parser {

    struct HttpResponse{
        int statuscode;
        int httpMajorVersion;
        int httpMinorVersion;

        std::string statusMessage;

        std::map<std::string,std::string>headers;
        std::vector<char> body;

        void clear(){
            statuscode = 0;
            httpMajorVersion = 0;
            httpMinorVersion = 0;

            statusMessage.clear();
            headers.clear();
            body.clear();
        }
    };
}