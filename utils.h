#ifndef LINUX_LIB_H
#define LINUX_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/syscall.h>

#define LOG(fmt, ...) \
    debug_log(__FUNCTION__, __LINE__, fmt, ## __VA_ARGS__);

typedef void handler_t(int);

//inner tools
int tcp_socket();
void send_error(int connfd, const char* info);
void unix_error(const char* msg);

//outer interface
ssize_t rio_readn(int fd, const void* usr_buf, size_t n);
ssize_t rio_writen(int fd, const void* usr_buf, size_t n);
int set_nonblocking(int fd);

//signal Tools
void sig_handler(int sig, int fd);
void reg_sig(int signum, handler_t* handler, bool restart);

//sockets tool and other
int get_listen_socket(const char* port, int queue_size);
int get_client_socket(const char* ip_address, const char* port);
int accept_connection(struct sockaddr_in* clnt_addr, int serv_sock);
void ignore_time_wait(int sock);

//logger
void debug_log(const char* fun, int line_no, const char* format, ...);

#endif
