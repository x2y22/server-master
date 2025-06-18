#include"head.h"

// 最大的文件描述符个数（后期需进行压力测试）
#define MAX_FD 65535
// 最大的监听事件数量
#define MAX_EVENT_NUMBER 10000

// 添加信号捕捉
void addsig(int sig, void(handler)(int)){
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

int main(int argc, char * argv[]){
    if(argc <= 1){
        printf("按照如下格式运行： %s port_number\n", basename(argv[0]));
        // exit(-1);
        return 1;
    }

    // 获取端口号
    int port = atoi(argv[1]);

    // 对SIGPIPE信号处理
    addsig(SIGPIPE, SIG_IGN);

    // 创建线程池
    threadpool<http_conn> * pool = NULL;
    try{
        pool = new threadpool<http_conn>;
    }catch(...){
        // exit(-1);
        return 1;
    }

    // 创建一个数组用于保存所有客户端信息
    http_conn * users = new http_conn[MAX_FD];

    // 创建监听套接字
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);

    // 设置端口复用（一定是在绑定之前设置）
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // 绑定
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    int ret = 0;
    ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));

    // 监听
    ret = listen(listenfd, 5);

    // 创建epoll对象、事件数组
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);

    // 将监听的文件描述符添加到epoll对象中
    addfd(epollfd, listenfd, false);
    http_conn::m_epollfd = epollfd;

    while(true){
        int num = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if(num < 0 && errno != EINTR){
            printf("epoll failure\n");
            break;
        }

        // 循环遍历事件数组
        for(int i = 0; i < num; ++i){
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd){
                // 有客户连接进来

                printf("New client connected.\n");

                struct sockaddr_in client_address;
                socklen_t client_addrlen = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlen);

                if(http_conn::m_user_count >= MAX_FD){
                    // 目前连接数满了
                    close(connfd);
                    continue;
                }

                // 将新客户的数据初始化放到数组中
                users[connfd].init(connfd, client_address);
            }else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                // 对方异常断开或者错误等事件
                users[sockfd].close_conn();
            }else if(events[i].events & EPOLLIN){

                printf("Reading data from client %d\n", sockfd);

                if(users[sockfd].read()){
                    // 一次性把所有数据都读完
                    pool->append(users + sockfd);
                }else{
                    users[sockfd].close_conn();
                }
            }else if(events[i].events & EPOLLOUT){

                printf("Writing response to client %d\n", sockfd);

                // 一次性写完所有数据
                if(!users[sockfd].write()){ // 写失败
                    users[sockfd].close_conn();
                }
            }
        }
    }

    close(epollfd);
    close(listenfd);
    delete [] users;
    delete pool;

    // system("pause");
    return 0;
}
