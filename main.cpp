#include "include/webCrawler.hpp"
#include<iostream>


int main(int argv,char** argc){

    if(argv<2){
        std::cerr<<"Not Provide SeedUrl and Maxdepth"<<std::endl;
        return 1;
    }

    int maxDepth = 2;
    std::string seedUrl = argc[1];
    if(argv>=2){
        maxDepth = std::stoi(argc[2]);
    }


    std::cout << "Starting web crawler at " << seedUrl << " with max depth " << maxDepth << std::endl;

    crawler::WebCrawler crawl(maxDepth,true);
    crawl.start(seedUrl);
    return 0;

}