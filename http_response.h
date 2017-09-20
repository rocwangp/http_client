#pragma once

#include <string>
#include <sstream>

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <cstring>
#include <cstdio>

class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();
    
    //used by client 
    void parse_response(const std::string& response);
    std::string get_location() const;
    int get_status_code() const;
    long long int get_content_length() const;
    
    //used by server
    std::string get_http_response(int status_code, const std::string& filename);
    
private:
    void parse_status_code(const std::string& response);
    void parse_location(const std::string& response);
    void parse_content_length(const std::string& response);

    std::string get_response_200(const std::string& filename);
    std::string get_response_302();
    std::string get_response_400();

private:
    std::string m_location; //if redirection, response will contain a new url
    int m_status_code;  //used to judge request status, 200, 302 or 404...
    long long int m_content_length; //when download a file from server, response will contain file size
};
