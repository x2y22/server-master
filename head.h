#ifndef HEAD_H
#define HEAD_H

#include<pthread.h>
#include<exception>
#include<semaphore.h>
#include<list>
#include<cstdio>
#include<stdlib.h>
#include<stdarg.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<cassert>
#include<errno.h>
#include<fcntl.h>
#include<signal.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<sys/uio.h>


#include"locker.h"
#include"threadpool.h"
#include"http_conn.h"

#endif
