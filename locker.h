#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

//semaphore package
class Semaphore
{
public:
    Semaphore()
    {
      if (sem_init(&m_sem, 0, 0) != 0)
      {
          throw std::exception();
      }
    }

    ~Semaphore() { sem_destroy(&m_sem); }

    bool wait() { return sem_wait(&m_sem) == 0; }
    bool post() { return sem_post(&m_sem) == 0; }

private:
    sem_t m_sem;
};

//mutex package
class Locker
{
public:
    Locker()
    {
      if (pthread_mutex_init(&m_mutex, NULL) != 0)
      {
          throw std::exception();
      }
    }

    ~Locker() { pthread_mutex_destroy(&m_mutex); }
    bool lock() { return pthread_mutex_lock(&m_mutex) == 0; }
    bool unlock() { return pthread_mutex_unlock(&m_mutex) == 0; }

private:
    pthread_mutex_t m_mutex;
};

/*condition variable package, condition variable is used for solving the problem that
    wasting CPU time because of polling we desired item that is not ready by using mutex.

    that is, condition variable can make a event that will wakeup a sleeping thread for waiting a
    desired item
*/
class ConditionVar
{
public:
    ConditionVar()
    {
      if (pthread_mutex_init(&m_mutex, NULL) != 0)
      {
          throw std::exception();
      }
      if (pthread_cond_init(&m_cond, NULL) != 0)
      {
          pthread_mutex_destroy(&m_mutex);
          throw std::exception();
      }
    }

    ~ConditionVar()
    {
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_cond);
    }

    bool wait()
    {
        int ret = 0;
        pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_cond, &m_mutex);
        pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }

    bool signal()
    {
        return pthread_cond_signal(&m_cond) == 0;
    }

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};

#endif
