/*helf-sync/helf-reactive mode thread pool*/
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"
#include "utils.h"

struct thread_pool_task 
{
    void* (*func)(void*);
    void* args;
};

class ThreadPool
{
public:
    ThreadPool(int thread_num = 8, int max_requests = 10000);
    ~ThreadPool();

    bool append(void* (*func)(void*), void *args); //add work to queue
    void run();                        //one loop per thread
    static void* worker(void* arg);

private:
    int m_thread_number;    //number of threads
    int m_max_requests;     //max size of work queue
    pthread_t* m_threads;   //thread id array
    std::list<thread_pool_task*> m_workqueue;  //work queue
    Locker m_queuelocker;       //mutex for work queue
    Semaphore m_queue_stat;           //sem for work queue indicate how many works in queue
    bool m_stop;            //if we stop thread?
};


#endif
