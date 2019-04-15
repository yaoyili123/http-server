#include "../subreactor.h"
#define BUF_SIZE 256

SubReactor sr(0);
int SubReactor::epfd = 0;   //defination of static member

void set_buf(char* buf) 
{
    sr.scan_idx = 0;
    sr.read_idx = strlen(buf);
    strcpy(sr.buf, buf);
}

void test_parse_line()
{
    sr.init();
    char case1[BUF_SIZE] = "sadasd\r\n";
    char case2[BUF_SIZE] = "sadasd";
    char case3[BUF_SIZE] = "sadasd\r";
    char case4[BUF_SIZE] = "sadasd\n";
    char case5[BUF_SIZE] = "sadasd\rsdadasd";

    set_buf(case1);
    assert(sr.parse_line() == LINE_OK);
    assert(sr.scan_idx == sr.read_idx);

    set_buf(case2);
    assert(sr.parse_line() == LINE_AGAIN);
    assert(sr.scan_idx == sr.read_idx);

    set_buf(case3);
    assert(sr.parse_line() == LINE_AGAIN);
    assert(sr.scan_idx == sr.read_idx);
    strcat(sr.buf, "\n");
    sr.read_idx++;
    assert(sr.parse_line() == LINE_OK);

    set_buf(case4);
    assert(sr.parse_line() == LINE_ERROR);
    // assert(sr.scan_idx == sr.read_idx);

    set_buf(case5);
    assert(sr.parse_line() == LINE_ERROR);
    // assert(sr.scan_idx == sr.read_idx);
    
}

void test_parse_request_line() 
{
    sr.init();
    char case1[BUF_SIZE] = "GET http://yaoyili/index.html HTTP/1.1";
    char case2[BUF_SIZE] = "POST http://yaoyili/index.html HTTP/1.1";
    char case3[BUF_SIZE] = "GET";
    char case4[BUF_SIZE] = "GET ftp://ssss ";
    char case5[BUF_SIZE] = "GET http://yaoyili HTTP/1.1";
    char case6[BUF_SIZE] = "GET http://yaoyili/index.html HTTP/1.0";
    char case7[BUF_SIZE] = "GET http://yaoyili/index.html?wzs=sb HTTP/1.1";
    char case8[BUF_SIZE] = "GET http://yaoyili/index.html?wzs=sb&ryh=sb HTTP/1.1";
    char case9[BUF_SIZE] = "GET http://yaoyili/index.html?wzs HTTP/1.1";

    set_buf(case1);
    assert(sr.parse_request_line(case1) == PROCESS_NEXT);
    // assert(sr.scan_idx == sr.read_idx);
    assert(!strcmp(sr.url, "/index.html"));
    assert(sr.version == "HTTP/1.1");
    assert(sr.domain_name == "yaoyili");

    set_buf(case2);
    assert(sr.parse_request_line(case2) == NOT_IMPLEMENTED);

    set_buf(case3);
    assert(sr.parse_request_line(case3) == BAD_REQUEST);

    set_buf(case4);
    assert(sr.parse_request_line(case4) == BAD_REQUEST);

    set_buf(case5);
    assert(sr.parse_request_line(case5) == PROCESS_NEXT);
    assert(sr.domain_name == "/index.html");

    set_buf(case6);
    assert(sr.parse_request_line(case6) == BAD_REQUEST);

    set_buf(case7);
    assert(sr.parse_request_line(case7) == PROCESS_NEXT);
    // assert(sr.parse_query_params() == 1);
    assert(sr.params.size() == 1);
    assert(sr.params["wzs"] == "sb");

    set_buf(case8);
    assert(sr.parse_request_line(case8) == PROCESS_NEXT);
    // assert(sr.parse_query_params() == 2);
    assert(sr.params.size() == 2);
    assert(sr.params["wzs"] == "sb");
    assert(sr.params["ryh"] == "sb");

    set_buf(case9);
    assert(sr.parse_request_line(case9) == BAD_REQUEST);
}

void test_parse_headers() 
{
    char case1[BUF_SIZE] = "Content-length: 15040";
    char case2[BUF_SIZE] = "Date: Tue, 3 Oct 1997 02:16:03 GMT";
    char case3[BUF_SIZE] = "Accept: image/gif, image/jpeg, text/html";
    char case4[10] = "\0";
    char case5[BUF_SIZE] = "Accept";
    char case6[BUF_SIZE] = "Accept:";
    char case7[BUF_SIZE] = "Accept: ";

    assert(sr.parse_headers(case2) == PROCESS_NEXT);
    assert(sr.headers["Date"] == "Tue, 3 Oct 1997 02:16:03 GMT");

    assert(sr.parse_headers(case3) == PROCESS_NEXT);
    assert(sr.headers["Accept"] == "image/gif, image/jpeg, text/html");

    assert(sr.parse_headers(case4) == OK_404);

    assert(sr.parse_headers(case1) == PROCESS_NEXT);
    assert(sr.headers["Content-length"] == "15040");

    assert(sr.parse_headers(case4) == PROCESS_NEXT);
    assert(sr.parse_state == CONTENT);

    assert(sr.parse_headers(case5) == BAD_REQUEST);
    assert(sr.parse_headers(case6) == BAD_REQUEST);
    assert(sr.parse_headers(case7) == BAD_REQUEST);
}

int main(int argc, char const *argv[])
{
    // test_parse_line();
    // test_parse_request_line();
    test_parse_headers(); 
    return 0;
}
