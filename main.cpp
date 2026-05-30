#include "include/webCrawler.hpp"
#include<iostream>


int main(int argc, char** argv) {

    if(argc < 2) {
        std::cerr << "Please provide SeedUrl and optionally MaxDepth" << std::endl;
        return 1;
    }

    int maxDepth = 2;
    std::string seedUrl = argv[1];
    
    if(argc >= 3) {
        maxDepth = std::stoi(argv[2]);
    }

    std::cout << "Starting web crawler at " << seedUrl << " with max depth " << maxDepth << std::endl;

    crawler::WebCrawler crawl(maxDepth, true);
    crawl.start(seedUrl);
    return 0;

}