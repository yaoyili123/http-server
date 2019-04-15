#ifndef EVENT_POLL
#define EVENT_POLL
#include "utils.h"
#define MAX_EVENT_NUMBER 10000
#define LISTEN_QUEUE 1024

int epoll_init();
int epoll_add(int epfd, int fd, void* req, __uint32_t events);
int epoll_add(int epfd, int fd, __uint32_t events);
int epoll_del(int epfd, int fd);
int epoll_mod(int epfd, int fd, __uint32_t events);
int epoll_mod(int epfd, int fd, void *req, __uint32_t events);
int wait_events(int epfd, epoll_event* events, int maxevents, int timeout);
void close_conn(int epfd, int sock);

#endif
