#pragma once

#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>

#include <unistd.h>
#include <cstring>

#define SOCKET_BUFFER_SIZE 4096

class Socket
{
public:
    Socket();
    ~Socket();

    bool CreateSocket();
    bool CloseSocket();
    
    bool Bind(const std::string& host, int port);
    bool Listen(const int listen_num = 10);
    
    int Accept();

    bool Connect(const std::string& host, int port);
    
    int Send(const std::string &message);
    int Recv(std::string& message);
    bool Recv_Char(std::string& message);
    
    int GetSocket() { return m_sockfd; }
private:
    int m_sockfd;
};
