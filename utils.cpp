#include "utils.h"

void unix_error(const char *msg)
{
    perror(msg);
    exit(1);
}

void debug_log(const char* fun, int line_no, const char* format, ...)
{
    char head[100];
    snprintf(head, sizeof(head),"[thread:%ud] %s():%d : ",
        pthread_self(), fun, line_no);

    va_list arg_list;
    va_start(arg_list, format);
    va_list arg_list2;
    va_copy(arg_list2, arg_list);
    char msg[vsnprintf(NULL, 0, format, arg_list) + 1];
    va_end(arg_list);
    vsnprintf(msg, sizeof(msg), format, arg_list2);
    va_end(arg_list2);

    printf("[DEBUG] %s %s\n", head, msg);
}

int tcp_socket()
{
    int sock;
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) 
    {
        unix_error("socket() error");
    }
    ignore_time_wait(sock);
    return sock;
}

void send_error(int connfd, const char* info)
{
    printf("%s\n", info);
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

int get_listen_socket(const char* port, int queue_size)
{
    int sock;
    struct sockaddr_in serv_addr;

    sock = tcp_socket();
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(port));

    ignore_time_wait(sock);

    if (bind(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
    {
        unix_error("bind() error");
    }

    if (listen(sock, queue_size) == -1)
    {
        unix_error("listen() error");
    }

    return sock;
}

int get_client_socket(const char* ip_address, const char* port)
{
    int sock;
    struct sockaddr_in clnt_addr;

    sock = tcp_socket();
    memset(&clnt_addr, 0, sizeof(clnt_addr));
    clnt_addr.sin_family = AF_INET;
    clnt_addr.sin_addr.s_addr = inet_addr(ip_address);
    clnt_addr.sin_port = htons(atoi(port));

    if (connect(sock, (struct sockaddr*) &clnt_addr, sizeof(clnt_addr)) == -1)
    {
        unix_error("connect() error");
    }
    else
        LOG("Connected")

    return sock;
}

int accept_connection(struct sockaddr_in* clnt_addr, int serv_sock)
{
    int clnt_sock;
    socklen_t clnt_addr_size;

    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*) clnt_addr, &clnt_addr_size);

    return clnt_sock;
}

void ignore_time_wait(int sock)
{
    int option = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
}

ssize_t rio_readn(int fd, const void* usr_buf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    ssize_t read_bytes = 0;
    char* buf = (char*)usr_buf;

    while (nleft > 0) {
        if ((nread = read(fd, buf, nleft)) < 0) {
            if (errno == EINTR) //interrupted by sig handler
                nread = 0;
            else if (errno == EAGAIN || errno == EWOULDBLOCK)
                return read_bytes;
            else
                return -1;
        }
        else if (nread == 0)
            break;
        nleft -= nread;
        buf += nread;
        read_bytes += nread;
    }
    return read_bytes;
}

ssize_t rio_writen(int fd, const void* usr_buf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    ssize_t write_bytes = 0;
    char* buf = (char*)usr_buf;

    while (nleft > 0)
    {
        if ((nwritten = write(fd, buf, nleft)) < 0)
        {
            if (errno == EINTR) //interrupted by sig handler
                nwritten = 0;
            else if (errno == EAGAIN || errno == EWOULDBLOCK)
                return write_bytes;
            else
                return -1;
        }
        else if (nwritten == 0)
            break;
        nleft -= nwritten;
        buf += nwritten;
        write_bytes += nwritten;
    }
    return write_bytes;
}

int set_nonblocking(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) == -1) 
    {
        perror("call fcntl error");
        return -1;
    }
    return 0;
}

//signal tools, unify events of source
//send signal to main loop
void sig_handler(int sig, int fd)
{
    //保证可重入性
    int save_errno = errno;
    int msg = sig;
    send(fd, (char*) &msg, 1, 0);
    errno = save_errno;
}

void reg_sig(int signum, handler_t* handler, bool restart)
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    if (restart)
    {
        act.sa_flags |= SA_RESTART;
    }
    sigfillset(&act.sa_mask);   //防止重复signal
    if (sigaction(signum, &act, 0) < 0)
    {
        unix_error("Signal error");
    }
}
