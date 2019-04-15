#include "../utils.h"
#include "../epoll.h"

static const char *msg = "yaoyili is a walker";

//FIXME: 为什么我从epoll_events中把fd删除了，它的EPOLLOUT事件还是会触发？

void start_conn(int epfd, int num, const char *port)
{
    struct sockaddr_in addr;
    int sock;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(atoi(port));

    for (int i = 0; i < num; i++)
    {
        sleep(1);
        sock = tcp_socket();
        LOG("create 1 socket");
        if (sock < 0)
        {
            continue;
        }
        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == 0)
        {
            LOG("build Connection");
            epoll_add(epfd, sock, EPOLLOUT | EPOLLET);
            set_nonblocking(sock);
        }
    }
}

void close_conn(int epfd, int sock)
{
    epoll_del(epfd, sock);
    close(sock);
}

bool read_once(int sock, char *buf, int len)
{
    int read_bytes = 0;
    memset(buf, 0, strlen(buf));
    read_bytes = read(sock, buf, len);
    if (read_bytes == -1 || read_bytes == 0)
    {
        return false;
    }
    LOG("read complete %d bytes: %s", read_bytes, buf);
    return true;
}

bool write_nbytes(int sock, const char *buf, int len)
{
    int write_byte = rio_writen(sock, buf, len);
    // LOG("write %d bytes\n", write_byte);
    if (write_byte == -1 || write_byte < len)
        return false;
    return true;
}

int main(int argc, char const *argv[])
{
    assert(argc == 3);

    //use I/O multiplexing
    int epfd = epoll_create(100);
    //arg1: port number, arg2: amount of conns
    start_conn(epfd, atoi(argv[2]), argv[1]);
    epoll_event events[10000];
    char buf[2048];

    while (1)
    {
        int fds = epoll_wait(epfd, events, 10000, 2000);
        for (int i = 0; i < fds; i++)
        {
            int sock = events[i].data.fd;
            if (events[i].events & EPOLLIN)
            {
                LOG("Connected socket %d read", sock);
                if (!read_once(sock, buf, 2048))
                {
                    LOG("socket %d read fail, close Connection %s", sock, strerror(errno));
                    close_conn(epfd, sock);
                    continue;
                }
                epoll_mod(epfd, sock, EPOLLOUT | EPOLLET);
            }
            else if (events[i].events & EPOLLOUT)
            {
                LOG("Connected socket write");
                if (!write_nbytes(sock, msg, strlen(msg)))
                {
                    LOG("socket %d write fail, close Connection %s", sock, strerror(errno));
                    close_conn(epfd, sock);
                    continue;
                }
                LOG("socket %d write complete bytes: %d", sock, strlen(msg))
                epoll_mod(epfd, sock, EPOLLIN | EPOLLET);
            }
            else if (events[i].events & EPOLLERR)
            {
                LOG("Connected socket %d err %s", sock, strerror(errno));
                close_conn(epfd, sock);
            }
        }
    }
    return 0;
}
