#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include"head.h"

void addfd(int epollfd, int fd, bool one_shot);
void removefd(int epollfd, int fd);
void modfd(int epollfd, int fd, int ev);


class http_conn{
public:
    // 所有socket事件都被注册到同一个socket对象上
    static int m_epollfd; 
    // 统计用户的数量     
    static int m_user_count;    

    http_conn(){}
    ~http_conn(){}

    void init(int sockfd, const sockaddr_in& addr); // 初始化新接受的连接
    void close_conn();  // 关闭连接
    void process(); // 处理客户端请求
    bool read();// 非阻塞读
    bool write();// 非阻塞写

private:
    int m_sockfd; // 该HTTP连接的socket
    sockaddr_in m_address; // 通信的socket地址

};

#endif
