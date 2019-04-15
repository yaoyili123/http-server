#include "subreactor.h"

void SubReactor::process()
{
    LOG("start process socket : %d", clnt_sock)
    init();

    //由于read事件整个subreactor的process就触发一次，因此就只需要读一次
    int read_bytes;
    if ((read_bytes = read_data()) < 0) 
    {
        error_io(); 
        return;
    }
    if ((read_idx + read_bytes) >= BUF_SIZE) 
    {
        error_io();
        return;
    }
    read_idx += read_bytes;

    //processing
    //TODO: 处理当parse_line返回LINE_AGAIN时的情况

    // int write_len = rio_writen(clnt_sock, msg, read_bytes);
    // if (write_len < 0)
    // {
    //     LOG("write fail: %s", strerror(errno))
    // }
    // __uint32_t flags = EPOLLIN | EPOLLRDHUP | EPOLLONESHOT | EPOLLET;
    // //传null是个错误的想法，因为每次都注册了新的event对象
    // epoll_mod(epfd, clnt_sock, (void *)this, flags);
}

void SubReactor::init()
{
    read_idx = scan_idx = 0;
    parse_state = PARSE_STATE::REQUEST_LINE;
    memset(buf, 0, sizeof(char) * BUF_SIZE);
}

void SubReactor::error_io()
{
    close_conn(epfd, clnt_sock);
    delete this;
}

int SubReactor::read_data()
{
    int read_bytes = 0;
    while (true)
    {
        int len = rio_readn(clnt_sock, buf + read_idx, BUF_SIZE);
        if (len < 0)
        {
            LOG("read fail because of %s", strerror(errno))
            return -1;
        }
        else if (len == 0)
        {
            LOG("read 0 bytes %s", strerror(errno))
        }

        read_bytes += len;
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            break;
        }
    }
    return read_bytes;
}

HTTP_RESULT SubReactor::parse_request()
{
    LINE_STATUS line_res = LINE_AGAIN;
    PARSE_STATE tmp_state;
    HTTP_RESULT parse_res;

    while ((line_res = parse_line()) == LINE_OK) 
    {
        //获取要parsing的字符串
        char *temp = get_start();
        start_idx = scan_idx; 
        tmp_state = parse_state;
        switch (tmp_state) 
        {
            case REQUEST_LINE: 
            {
                parse_res = parse_request_line(temp);
                if (parse_res == BAD_REQUEST) 
                {
                    return BAD_REQUEST;
                }
            }
            case HEADERS:
            {
                parse_res = parse_headers(temp);
                if (parse_res == BAD_REQUEST || parse_res == OK_404)
                {
                    return parse_res;
                }
            }
            case CONTENT:

        }
    }
    return PROCESS_NEXT;
}

LINE_STATUS SubReactor::parse_line()
{
    for (; scan_idx < read_idx; ++scan_idx)
    {
        char temp = buf[scan_idx];
        if (temp == '\r')
        {
            if (scan_idx + 1 >= read_idx) 
            {
                scan_idx++;
                return LINE_AGAIN;
            }
            else if (buf[scan_idx + 1] == '\n')
            {
                buf[scan_idx++] = '\0';
                buf[scan_idx++] = '\0';
                return LINE_OK;
            }
            else 
            {
                return LINE_ERROR;
            }
        }
        if (temp == '\n')
        {
            if (scan_idx > 1 && buf[scan_idx - 1] == '\r')
            {
                buf[scan_idx - 1] = '\0';
                buf[scan_idx++] = '\0';
                return LINE_OK;
            }
            else
            {
                return LINE_ERROR;
            }
        }
    }
    return LINE_AGAIN;
}

/*
    ret:
    0，don't have query params
    -1 format error
    >0, number of query params
*/
int SubReactor::parse_query_params()
{
    if (!url) return 0;

    char *head;
    if ((head = strpbrk(url, "?")) == NULL)
    {
        return 0;
    }
    
    char *key, *value;
    int cnt = 1;
    bool end = false;
    while (true) 
    {
        key = ++head;
        if ((head = strpbrk(head, "=")) == NULL)
        {
            return -1;
        }
        *head++ = '\0';
        value = head;
        if ((head = strpbrk(head, "&")) == NULL) 
        {
            end = true;
        } 
        else 
        {
            *head = '\0';
        }
        params[std::string(key)] = std::string(value);
        if (end) break;
        cnt++;
    }
    return cnt;
}

HTTP_RESULT SubReactor::parse_request_line(char* text)
{
    char *start = strpbrk(text, " \t"); //url
    //没有url
    if (start == NULL) 
    {
        return BAD_REQUEST;
    }
    *start++ = '\0';
    if (strncmp(text, "GET", 3)) 
    {
        return NOT_IMPLEMENTED;
    }
    method = GET;
    
    start += strspn(start, " \t");    //start->url
    char *end = strpbrk(start, " \t");  //end url

    //check url format
    if (strncmp(start, "http://", 7)) 
    {
        return BAD_REQUEST;
    }
    start += 7;
    
    //get domain_name
    //如果是这样的“yaoyili HTTP/1.1”的串，搜索符只有"/"不足以判断它没有URI
    char* domain_end = strpbrk(start, " /");
    if (domain_end == NULL || (*domain_end) == ' ')
    {
        // strcpy(domain_name, "/index.html");
        domain_name = std::string("/index.html");
        url = NULL;
    }
    else 
    {
        // strncpy(domain_name, start, (domain_end - start));
        // domain_name = start;
        domain_name = std::string(start, (domain_end - start));
        //get url
        start = domain_end;
        url = start;
        *end++ = '\0';
    }

    //version
    start = end;
    start += strspn(start, " \t");
    if (strncmp(start, "HTTP/1.1", 8))
    {
        return BAD_REQUEST;
    }
    version = std::string("HTTP/1.1");
        // strcpy(version, "HTTP/1.1"); 没有分配空间的char*不能用strcpy
    start += 8;

    if (parse_query_params() == -1) 
    {
        return BAD_REQUEST;
    }
    parse_state = HEADERS;
    // scan_idx += start - text;
    return PROCESS_NEXT;
}

HTTP_RESULT SubReactor::parse_headers(char *text)
{
    char *start = text;
    char *key, *value;
    HEADER_PARSE state = H_START;

    while (true) 
    {
        switch (state)
        {
            case H_START: 
            {
                //说明headers已经结束了
                if (*start == '\0')
                {
                    if (headers.count("Content-length") > 0) 
                    {
                        parse_state = CONTENT;
                        return PROCESS_NEXT;
                    }
                    else 
                    {
                        return OK_404;
                    }
                }
                else 
                {
                    key = start;
                    state = H_KEY;
                }
            }
            break;
            case H_KEY: 
            {
                if (*start == ':') 
                {
                    state = H_COLON;
                    *start = '\0';
                }
                else if (*start == '\0') 
                {
                    return BAD_REQUEST;
                }
            }
            break;
            case H_COLON:
            {
                if (*start == ' ') 
                {
                    state = H_SPACE_AFTER_COLON;
                }
                else 
                {
                    return BAD_REQUEST;
                }
            }
            break;
            case H_SPACE_AFTER_COLON:
            {
                if (*start == '\0') 
                {
                    return BAD_REQUEST;
                }
                state = H_VALUE;
                value = start;
            }
            break;
            case H_VALUE:
            {
                if (*start == '\0')
                {
                    headers[key] = value;
                    state = H_CRLF;
                }
            }
            break;
            case H_CRLF:
            {
                return PROCESS_NEXT;
            }
            break;
        }
        start++;
    }
    return BAD_REQUEST;
}
