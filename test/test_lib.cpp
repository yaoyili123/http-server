#include "../utils.h"

void test_log()
{
    const char *name = "峰哥";
    LOG("wzs is a dog")
    LOG("%s fuck wzs", name)
}

void test_write()
{
    char buf[30] = "森哥 我TM射爆\n";
    int len = strlen(buf);
    // int len1 = rio_writen(STDOUT_FILENO, buf, strlen(buf));
    // printf("len: %d %d\n", len, len1);
    assert(rio_writen(STDOUT_FILENO, buf, strlen(buf)) == len);
}

void test_read()
{
    char buf[30];
    assert(rio_readn(STDIN_FILENO, buf, sizeof(buf)) == 7);
}

int main(int argc, char const *argv[])
{
    
    test_log();
    test_write();
    test_read();
    
    return 0;
}
