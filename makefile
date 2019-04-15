#通用性强的Makefile

#wildcard function, 表示这个有wildcard的变量将会用于target和prerequisites中
#其中的*.cpp是一种match exp，因此也被称为pattern， 它会被parse成多个以space分隔的list
SOURCE := $(wildcard *.cpp)
#patsubst函数，用于pattern string replacement, %是匹配任意数量的任意字符
OBJS := $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(SOURCE)))

TARGET := http-server
CC := g++
INCLUDE :=
LIBS := -lpthread
CFLAGS := -std=c++11 -g -Wall -O3 $(INCLUDE)

#特殊target，表明make会无条件执行其prerequisites中的phony targets
.PHONY:  objs co cr rb all
all: $(TARGET)
objs: $(OBJS)
#rebuild
rb: ct co all
co:
	rm -fr *.o
ct:
	rm -rf $(TARGET)

#linking
#'$@'是一种Automatic Variables, 它指该rule的target的文件名称，无论target到底是什么
$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

main.o : main.cpp epoll.o subreactor.h threadpool.o
	$(CC) $(CFLAGS) -c main.cpp

epoll.o: epoll.h epoll.cpp
	$(CC) $(CFLAGS) -c epoll.cpp

threadpool.o: threadpool.h threadpool.cpp utils.h locker.h
	$(CC) $(CFLAGS) -c threadpool.cpp

utils.o: utils.cpp utils.h
	$(CC) $(CFLAGS) -c utils.cpp
	
