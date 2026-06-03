#include "../include/threadpool.hpp"
#include <cstddef>
#include <functional>
#include <mutex>
#include <utility>

namespace crawler {

    ThreadPool::ThreadPool(size_t threads) : stop(false),active_task(0){

        for(size_t i=0;i < threads;i++){
            workers.emplace_back([this](){
               while(true){
                std::function<void()>t;
                {
                    std::unique_lock<std::mutex> lock(this->mut);
                    this->cv.wait(lock,[this](){return this->stop || !this->task.empty();});

                    if(this->stop && this->task.empty())return;

                    t = std::move(this->task.front());
                    this->task.pop();
                    this->active_task++;
                }
                t();
                {
                    std::unique_lock<std::mutex> lock(mut);
                    this->active_task--;
                    if(this->active_task ==0 && this->task.empty()){
                        this->finshedCV.notify_all();
                    }
                }
               }
            });
        }
    }

    ThreadPool::~ThreadPool(){
        {
            std::unique_lock<std::mutex> lock(mut);
            stop = true;
        }
        cv.notify_all();
        for(auto& worker:workers){
            if(worker.joinable()){
                worker.join();
            }
        }
    }

    void ThreadPool::enqueue(std::function<void()>t){
        {
            std::unique_lock<std::mutex> lock(mut);
            task.emplace(std::move(t));
        }
        cv.notify_one();
    }

    void ThreadPool::waitFinshedTask(){
      std::unique_lock<std::mutex> lock(mut);
      finshedCV.wait(lock,[this]{
        return this->active_task==0 && this->task.empty();
      });


    }

}