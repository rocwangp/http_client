#pragma once


#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <cstdio>

#define HTTP_DEFAULT_BUFFER 1024

class HttpRequest
{
public:
    HttpRequest();
    ~HttpRequest();
    
    //used by client
    //download
    void parse_url(const std::string& url);
    std::string get_http_download_request() const;
    std::string get_ip_address() const;
    std::string get_filename() const;
    int get_port() const;
    
    //upload
    std::string get_http_upload_request(const std::string& filename); 
    std::string get_http_upload_header(const std::string& filename);
    std::string get_http_upload_tail();
    
    //used by server
    void parse_request(const std::string& request);
    std::string get_method() const;
    std::string get_argument() const;
    bool is_upload() const;
    long long int get_content_length() const;

private: 

    //used by client
    void parse_ip_address(const std::string& domain);
    //get filesize, used to generate upload request and upload header
  

    //used by server
    void parse_method(const std::string& request);
    void parse_filename_argument(const std::string& request);
    void parse_get_filename(const std::string& request);
    void parse_post_filename_argument(const std::string& request); 
    void parse_content_length(const std::string& request);
private:
    //used to generate download/upload request by client
    std::string m_url;  //save url from client
    std::string m_ip_address;   //parse ip_address from url 
    std::string m_filename; //parse the filename client need to access
    int m_port; //parse server_port from url, default 8080
    
    //used to parse the client_request by server
    std::string m_method;   //parse from request, GET/POST
    std::string m_argument; //POST method need argument, GET method is empty
    bool m_upload;  //POST :
                    // one : submit table
                    // two : upload file
    long long int m_content_length;
};
