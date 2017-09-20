#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include <cerrno>

#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "Socket.h"
#include "http_request.h"
#include "http_response.h"

#define DOWNLOAD_THREAD_NUM 10
#define DOWNLOAD_BUFFER_SIZE    4096
#define UPLOAD_BUFFER_SIZE      4096

#define HTTP_RESPONSE_OK        200
#define HTTP_RESPONSE_REDIR     302
#define HTTP_RESPONSE_BAD       400
#define HTTP_RESPONSE_NFOUND    404

class HttpClient
{
public:
    HttpClient();
    ~HttpClient();

    void download_file(const std::string& url);
    void upload_file(const std::string& url, const std::string& filename);

private:
   // bool Parse_URL(const std::string& url);
   // std::string Generate_Request_Header(const std::string& url);
    static void show_process_bar(long long int current_size, long long int total_size); 
    static void* process_download(void *arg);
    static void* process_upload(void *arg);
    
    //used to parse the filesize of upload file
    long long int parse_filesize(const std::string& filename);
private:
    Socket m_socket;   //package socket class
    HttpRequest m_http_request;
    HttpResponse m_http_response;

    std::string m_filename; //the filename of upload file, 
                            //used to generate upload request and upload header
  
};
