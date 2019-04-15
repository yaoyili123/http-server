#define TESTING
#include "../http_conn.h"

http_conn hc;
int request_size, read_fd;

void test_parse_line()
{
    /*1:24 2:22 3:23 4:21*/
    hc.parse_line();
    // printf("m_checked_idx=%d\n", hc.m_checked_idx);
    assert(hc.m_checked_idx == 26);
    // hc.m_checked_idx = 0;
    // hc.m_read_idx = 20;
    // char test[20] = "fuck you";
    // assert(http_conn::LINE_OPEN == hc.parse_line(test));

    hc.m_checked_idx = 0;
    hc.m_read_idx = request_size;
}

void test_process_read()
{
    hc.m_checked_idx = 0;
    hc.m_read_idx = request_size;
    http_conn::HTTP_CODE result = hc.process_read();
    // printf("process_read() result : %d\n", result);
    assert(result == http_conn::FILE_REQUEST);
}

void test_do_request()
{
    strcpy(hc.m_url, "/haha");
    assert(hc.do_request() == http_conn::NO_RESOURCE);
    strcpy(hc.m_url, "/dir");
    assert(hc.do_request() == http_conn::BAD_REQUEST);
}

void test_parse_request_line()
{
    char rl[50] = "GET /index.html HTTP/1.1";
    hc.parse_request_line(rl);
    assert(!strcmp(hc.m_method, "GET"));
    assert(!strcmp(hc.m_url, "/index.html"));
    assert(!strcmp(hc.m_version, "HTTP/1.1"));

    char r2[50] = "GET http://www.baidu,con/index.html HTTP/1.1";
    hc.parse_request_line(r2);
    assert(!strcmp(hc.m_method, "GET"));
    assert(!strcmp(hc.m_url, "/index.html"));
    assert(!strcmp(hc.m_version, "HTTP/1.1"));
}

void test_parse_headers()
{
    char header1[30] = "Host: www.yaoyili.com";
    char header2[30] = "Connection: keep-alive";
    char header3[30] = "Content_length: 1024";
    hc.parse_headers(header1);
    assert(strcmp(hc.m_host, "www.yaoyili.com") == 0);
    hc.parse_headers(header2);
    assert(hc.m_linger);
    hc.parse_headers(header3);
    assert(hc.m_content_length == 1024);
}

void init_write_buf()
{
    memset(hc.m_write_buf, 0, strlen(hc.m_write_buf));
    hc.m_write_idx = 0;
}

void test_process_write()
{
    http_conn::HTTP_CODE result = hc.process_read();
    // printf("process_read() result : %d\n", result);
    assert(result == http_conn::FILE_REQUEST);
    //testing code=FILE_REQUEST
    assert(hc.process_write(result));
    printf("FILE_REQUEST response: %s\n", hc.m_write_buf);
    init_write_buf();
    assert(hc.process_write(http_conn::FORBIDDEN));
    printf("FORBIDDEN response: %s\n", hc.m_write_buf);
    init_write_buf();
    assert(hc.process_write(http_conn::NO_RESOURCE));
    printf("NO_RESOURCE response: %s\n", hc.m_write_buf);
    init_write_buf();
    assert(hc.process_write(http_conn::BAD_REQUEST));
    printf("BAD_REQUEST response: %s\n", hc.m_write_buf);
    init_write_buf();
    assert(hc.process_write(http_conn::INTERNAL_ERROR));
    printf("INTERNAL_ERROR response: %s\n", hc.m_write_buf);
}

void init_test()
{
    hc.init();
    read_fd = open("http_request", O_RDONLY);
    assert(read_fd != -1);
    hc.m_read_idx = read(read_fd, hc.m_read_buf, http_conn::READ_BUFFER_SIZE);
    request_size = hc.m_read_idx;
    assert(hc.m_read_idx != -1);
    // printf("message length:%d\n", request_size);
}

void end_test()
{
    close(read_fd);
    printf("testing success\n");
}

int main(int argc, char const *argv[])
{
    init_test();
    //先进行业务逻辑（HTTP request parsing and build a response）的测试
    // test_parse_line();
    // test_parse_request_line();
    // test_parse_headers();
    // test_process_read();
    // test_do_request();
    test_process_write();

    end_test();
    return 0;
}
