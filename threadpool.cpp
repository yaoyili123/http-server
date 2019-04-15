#include"threadpool.h"

ThreadPool::ThreadPool(int thread_num, int max_requests) 
    : m_thread_number(thread_num), m_max_requests(max_requests),
    m_stop(false), m_threads(NULL)
{
    if ((thread_num <= 0) || (max_requests <= 0))
    {
        throw std::exception();
    }
    m_threads = new pthread_t[m_thread_number];
    if (!m_threads)
    {
        throw std::exception();
    }
    for (int i = 0; i < m_thread_number; ++i)
    {
        printf("create the %dth thread\n", i);
        if (pthread_create(m_threads + i, NULL, worker, this) != 0)
        {
            delete[] m_threads;
            throw std::exception();
        }
        //将所有的threads设置为detach mode, 使其exit时自动释放资源
        if (pthread_detach(m_threads[i]))
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
}

ThreadPool::~ThreadPool()
{
    delete[] m_threads;
    m_stop = true;
}

bool ThreadPool::append(void *(*func)(void *), void *args)
{
    m_queuelocker.lock();
    if (m_workqueue.size() >= m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    thread_pool_task* task = new thread_pool_task;
    task->func = func;
    task->args = args;
    m_workqueue.push_back(task);
    m_queue_stat.post();
    m_queuelocker.unlock();
    return true;
}

void *ThreadPool::worker(void *arg)
{
    ThreadPool *pool = (ThreadPool *)arg;
    LOG("thread: %ud start", pthread_self());
    pool->run();
    return pool;
}

void ThreadPool::run()
{
    while (!m_stop)
    {
        m_queue_stat.wait();
        m_queuelocker.lock();
        if (m_workqueue.empty())
        {
            m_queuelocker.unlock();
            continue;
        }
        thread_pool_task *task = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if (!task)
        {
            continue;
        }
        task->func(task->args);
        //应当由process决定是用epoll_event保存未完成的task，还是销毁
        // delete task;
    }
}
