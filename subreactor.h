#ifndef SUB_REACTOR
#define SUB_REACTOR

#include "utils.h"
#include "epoll.h"
#include <string>
#include <map>

enum METHOD
{
    GET = 0,
    POST,
    HEAD,
    PUT,
    DELETE,
    TRACE,
    OPTIONS,
    CONNECT,
    PATCH
};
//read a line state
enum LINE_STATUS
{
    LINE_OK = 0,
    LINE_ERROR,
    LINE_AGAIN
};
//parsing step state
enum PARSE_STATE
{
    REQUEST_LINE = 0,
    HEADERS,
    CONTENT,
};

enum HTTP_RESULT
{
    PROCESS_NEXT = 0,
    BAD_REQUEST,
    OK_404,
    NOT_IMPLEMENTED
};

enum HEADER_PARSE 
{
    H_START = 0,
    H_KEY,
    H_COLON,
    H_SPACE_AFTER_COLON,
    H_VALUE,
    H_CRLF,
};

//TODO: 每次添加任务都要new/delete，性能很差
class SubReactor
{
public:

    static const int BUF_SIZE = 1024;
    static int epfd; //这里只是声明

    SubReactor(int sock) : clnt_sock(sock) {}

    int get_fd() { return clnt_sock; }

    void process();         //reactor整个过程
    void init();            //初始化状态
    int read_data();        //处理reactor中的read部分
    void error_io();        //处理严重的I/O错误
    HTTP_RESULT parse_request(); //process过程，解析HTTP request
    char *get_start() { return buf + start_idx; }
    LINE_STATUS parse_line();
    int parse_query_params();
    HTTP_RESULT parse_request_line(char* text);
    HTTP_RESULT parse_headers(char* text);

    // private:
    int clnt_sock;
    char buf[BUF_SIZE];
    int read_idx;
    int scan_idx;
    int start_idx;
    PARSE_STATE parse_state;

    METHOD method;
    char* url;
    std::string version;
    std::string domain_name;
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> headers;
    std::string content;    
};

// int SubReactor::epfd = 0; //需要定义

#endif