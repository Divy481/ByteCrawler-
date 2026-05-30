#include "../include/url.hpp"
#include <algorithm>
#include<cctype>
#include <cstddef>
#include <optional>
#include <string>


namespace crawler {

    static void toLower(std::string& s){
        std::transform(s.begin(),s.end(),s.begin(),[](unsigned char ch){
            return std::tolower(ch);
        });
    }

    std::optional<Url> Url::parse(const std::string& urlString){

        //"http://example.com:8080/docs/index.html"

        
        Url url;
        std::string s = urlString;
        size_t schemaEnd = s.find("://");

        toLower(s);
        if(schemaEnd == std::string::npos){
            return std::nullopt;
        }

        url.schema = s.substr(0,schemaEnd);
        toLower(url.schema);
        if(url.schema !="http"){
            return std::nullopt;
        }

        url.port = "80";
        size_t hostStart = schemaEnd + 3;
        size_t pathStart = s.find('/',hostStart);

        std::string hostPath = "";
        if(pathStart == std::string::npos){
            hostPath = s.substr(hostStart);
            url.path ="/";
        }else{
            hostPath = s.substr(hostStart,pathStart - hostStart);
            url.path = s.substr(pathStart);
        }

        size_t portPos = hostPath.find(':');
        if(portPos == std::string::npos){
            url.host = hostPath;
        }else{
            url.host = hostPath.substr(0,portPos);
            url.port = hostPath.substr(portPos+1);
        }

        size_t fragmentPos = url.path.find('#');
        if(fragmentPos != std::string::npos){
            url.path = url.path.substr(0,fragmentPos);
        }

        return url;
    }

    std::optional<Url> Url::resolve(const Url& baseUrl, const std::string &link){
        //http://example.com/docs/index.html

        if(link.empty())return std::nullopt;

        if(link.find("://") != std::string::npos){
            return parse(link); //absloute  path 
        }
        if(link.substr(0,2) == "//"){
            return parse(baseUrl.schema+":"+ link);
        }

        Url newUrl = baseUrl;

        std::string tempLink = link;

        size_t framentPos = tempLink.find('#');

        if(framentPos !=std::string::npos){
            tempLink = tempLink.substr(0, framentPos);
        }

        if(tempLink.empty())return std::nullopt;

        if(tempLink[0] == '/'){
            newUrl.path = tempLink;
        }else{
            size_t lastSlash = newUrl.path.find_last_of('/');
            if(lastSlash!=std::string::npos){
                newUrl.path = newUrl.path.substr(0,lastSlash+1) + tempLink;
            }else{
                newUrl.path = "/" + tempLink;
            }
        }

        return newUrl;
    }

    std::string Url::toString() const {
        std::string res = schema + "://" + host;

        if((schema=="http" && port !="80") || (schema=="https" && port !="443")){
            res += ":" + port;
        }

        res += path;
        return res;

    }
}
