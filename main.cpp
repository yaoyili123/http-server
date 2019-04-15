#include "threadpool.h"
#include "epoll.h"
#include "subreactor.h"

extern struct epoll_event *events;

void* reactor_handler(void* args) 
{
    SubReactor *sr = (SubReactor*)args;
    sr->process();
    return 0;
}

void establish_connection(int listen_sock, int epfd, ThreadPool *pool)
{
    int clnt_sock = 0;
    struct sockaddr_in* cli_addr;
    socklen_t clnt_addr_size = sizeof(cli_addr);
    //listen_sock is non-blocking, then accpet don't blocking
    while ((clnt_sock = accept(listen_sock, (struct sockaddr *)cli_addr, &clnt_addr_size)) > 0)
    {
        int ret = set_nonblocking(clnt_sock);
        if (ret < 0)
        {
            LOG("error is %s", strerror(errno));
            return;
        }
        __uint32_t flags = EPOLLIN | EPOLLRDHUP | EPOLLONESHOT | EPOLLET;
        SubReactor *sr = new SubReactor(clnt_sock);
        // printf("fd: %d rc: %d\n", clnt_sock, sr->get_fd());
        // printf("accept rc pointer : %d\n", sr);
        epoll_add(epfd, clnt_sock, (void*)sr, flags);
    }
}

void handle_events(int events_num, int epfd, struct epoll_event *events, int listen_fd, ThreadPool *pool)
{
    for (int i = 0; i < events_num; i++)
    {
        SubReactor *sr = (SubReactor*)(events[i].data.ptr);
        // printf("event rc pointer : %d\n", sr);
        int sockfd = sr->get_fd();
        if (sockfd == listen_fd)
        {
            LOG("received connection")
            establish_connection(sockfd, epfd, pool);
        }
        else 
        {
            //EPOLLRDHUP用于判断peer是否close conn或者shutdown了output side
            if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) 
            {
                LOG("Connected socket: %d error %s", sockfd, strerror(errno))
                close_conn(epfd, sockfd);
                delete sr;
                continue;
            }
            LOG("socket %d read event", sockfd)
            // SubReactor *sr = new SubReactor(sockfd);
            // printf("fd: %d rc: %d\n", listen_fd, rc->get_fd());
            pool->append(reactor_handler, (void *)sr);
        }
    }
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("usage: %s <port-number>\n", argv[0]);
        exit(1);
    }
    //ignore SIGPIPE
    reg_sig(SIGPIPE, SIG_IGN, false);

    ThreadPool* pool = NULL;
    pool = new ThreadPool();
    int epfd = epoll_init();
    SubReactor::epfd = epfd;
    int listen_fd = get_listen_socket(argv[1], 5);
    //avoid accept blocking caller, 提高了响应能力
    set_nonblocking(listen_fd);
    SubReactor *rc = new SubReactor(listen_fd);
    // printf("fd: %d rc: %d\n", listen_fd, rc->get_fd());
    epoll_add(epfd, listen_fd, (void *)rc, EPOLLIN | EPOLLET);

    while (true)
    {
        int number = wait_events(epfd, events, MAX_EVENT_NUMBER, -1);
        // LOG("events number: %d", number)
        if (number == 0)
        {
            continue;
        }
        handle_events(number, epfd, events, listen_fd, pool);
    }

    close(epfd);
    close(listen_fd);
    delete pool;
    delete rc;
    return 0;
}
