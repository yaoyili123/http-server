#include "epoll.h"

struct epoll_event *events;

int epoll_init()
{
    int epfd = epoll_create(LISTEN_QUEUE);
    if (epfd == -1)
    {
        perror("call epoll_create_add error");
    }
    events = new epoll_event[MAX_EVENT_NUMBER];
    return epfd;
}

int epoll_add(int epfd, int fd, void* req, __uint32_t events)
{
    // LOG("call epoll_data(4 args)")
    struct epoll_event event;
    // SubReactor* sr = (SubReactor*)req;
    // LOG("epoll_add req fd: %d", sr->get_fd())
    // printf("epoll_add rc pointer : %d\n", req);
    event.data.ptr = req;
    event.events = events;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        perror("call epoll_ctl_add error");
        return -1;
    }
    return 0;
}

int epoll_add(int epfd, int fd, __uint32_t events)
{
    // LOG("call epoll_data(3 args)")
    struct epoll_event event;
    event.data.fd = fd;
    // LOG("call epoll_add socket : %d", fd)
    event.events = events;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        perror("call epoll_ctl_add error");
        return -1;
    }
    return 0;
}

int epoll_del(int epfd, int fd)
{
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) < 0)
    {
        perror("call epoll_ctl_del error");
        return -1;
    }
    return 0;
}

int epoll_mod(int epfd, int fd, __uint32_t events)
{
    //mod重新创建了一个event，所以要和add保持一致
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event) < 0)
    {
        perror("call epoll_ctl_mod error");
        return -1;
    }
    return 0;
}

int epoll_mod(int epfd, int fd, void *req, __uint32_t events)
{
    struct epoll_event event;
    event.data.ptr = req;
    event.events = events;
    if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event) < 0)
    {
        perror("call epoll_ctl_mod error");
        return -1;
    }
    return 0;
}

int wait_events(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
    int nums = epoll_wait(epfd, events, maxevents, timeout);
    if (nums < 0 && errno != EINTR) 
    {
        perror("call epoll_wait fail");
    }
    return nums;
}

void close_conn(int epfd, int sock)
{
    epoll_del(epfd, sock);
    close(sock);
}