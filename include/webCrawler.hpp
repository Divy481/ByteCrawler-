#pragma once
#include "url.hpp"
#include <queue>
#include <string>
#include <unordered_set>

namespace crawler {
    class WebCrawler{
        public:
            WebCrawler(int maxDepth=2,bool sameDomainOnly = true);
            void start(const std::string& seedUrl);

        private:
            struct CrawlerTask{
                Url url;
                int depth;
            };

            int maxDepth;
            bool sameDomainOnly;
            std::queue<CrawlerTask> task;
            std::unordered_set<std::string> visted;
            Url seedUrlObj;

            void processUrl(const Url& url,int currDepth);
            std::string fetch(const Url& url);
            std::vector<std::string> extractLink(const std::string& html);
            
    };
}