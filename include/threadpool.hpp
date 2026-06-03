#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>


namespace crawler {

    class ThreadPool{
        public:
            ThreadPool(std::size_t threads);
            ~ThreadPool();

            void enqueue(std::function<void()>task);
            void waitFinshedTask();

        private:
            std::mutex mut;
            std::condition_variable cv;
            
            std::queue<std::function<void()>> task;
            std::condition_variable finshedCV;

            std::vector<std::thread>workers;

            bool stop;
            std::atomic<size_t> active_task;


    };
}