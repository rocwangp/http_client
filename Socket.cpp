#include "Socket.h"

#include <iostream>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

Socket::Socket():
    m_sockfd(-1)
{

}


Socket::~Socket()
{
    if(m_sockfd != -1)
        CloseSocket();
}


bool Socket::CreateSocket()
{
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_sockfd < 0)
        return false;
    else
        return true;
}


bool Socket::CloseSocket()
{
    if(close(m_sockfd) < 0)
        return false;
    else
        return true;
}


bool Socket::Bind(const std::string &host, int port)
{
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(host.c_str());

    if(bind(m_sockfd, (struct sockaddr*)&address, sizeof(address)) < 0)
        return false;
    return true;
}


bool Socket::Listen(const int listen_num)
{
    if(listen(m_sockfd, listen_num) < 0)
        return false;
    else
        return true;
}


int Socket::Accept()
{
    return accept(m_sockfd, NULL, NULL);
}


bool Socket::Connect(const std::string& host,  int port)
{
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(host.c_str());

    if(connect(m_sockfd, (struct sockaddr*)&address, sizeof(address)) < 0)
        return false;
    else
        return true;
}



int Socket::Send(const std::string& message)
{
    return send(m_sockfd, message.c_str(), message.size(), MSG_NOSIGNAL);
}


int Socket::Recv(std::string& message)
{
    message.clear();
    char buffer[SOCKET_BUFFER_SIZE];
    bzero(buffer, SOCKET_BUFFER_SIZE);
    int ret = recv(m_sockfd, buffer, SOCKET_BUFFER_SIZE, 0);
    if(ret < 0)
        return -1;
    else if(ret == 0)
        return 0;
    else
    {
        message = buffer;
        return ret;
    }
}


bool Socket::Recv_Char(std::string& message)
{
    message.clear();
    char buffer[2] = {0};
    if(recv(m_sockfd, buffer, 1, 0) <= 0)
        return false;
    else
    {
        buffer[1] = '\0';
        message = buffer;
        return true;
    }
}
