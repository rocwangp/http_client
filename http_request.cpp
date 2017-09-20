#include "http_request.h"

HttpRequest::HttpRequest():
    m_url(""),
    m_ip_address(""),
    m_filename(""),
    m_port(-1),
    m_method(""),
    m_argument(""),
    m_upload(false),
    m_content_length(0)
{

}


HttpRequest::~HttpRequest()
{

}

//parse the url to obtain protocol, domain, port, filename and ip_address
void HttpRequest::parse_url(const std::string& url)
{ 
    m_url = url;
    
    //split http protocol if hash, then the front_idx point to the first_index of domain
    const std::string patterns[] = {"http://", "https://"};
    std::string::size_type front_idx = 0;
    for(int i = 0; i < 2; ++i)
    {
        if(url.substr(0, patterns[i].size()) == patterns[i])
        {
            front_idx = patterns[i].size();
            break;
        }
    }

    
    //split domain and port from url, from the first '/' to the second '/' is domain and port
    std::string::size_type back_idx = url.find("/", front_idx);
    if(back_idx == std::string::npos)
        back_idx = url.size();
    std::string domain = url.substr(front_idx, back_idx - front_idx);

    //split port from domain if has, from the first ':' to the last is port
    //if not, default port is 80
    back_idx = domain.find(":");
    if(back_idx != std::string::npos)
    {
        std::string portStr = domain.substr(back_idx + 1);
        domain = domain.substr(front_idx, back_idx - front_idx);
        sscanf(portStr.c_str(), "%d", &m_port);
    }
    else
    {
        m_port = 80;
    }

    //if the domain is website(like www.baidu.com), not ip_address(like 192,168.x.xxx), convert to it
    parse_ip_address(domain);
    
    //split filename need to access from url, from the last '/' to the last is filename
    
    front_idx = url.find("/", front_idx);
    if(front_idx == std::string::npos)
        m_filename = "";
    else
    {
        back_idx = url.find_last_of('?');
        if(back_idx == std::string::npos)
            back_idx = url.find_last_of('#');
        if(back_idx == std::string::npos)
            m_filename = url.substr(front_idx);
        else
            m_filename = url.substr(front_idx, back_idx - front_idx);
    }
     //filename with directory
  

#ifdef DEBUG
    std::cout << "parse url completed...\n" 
        << "protocol : " << m_protocol << "\n" 
        << "ip address : " << m_domain << "\n"
        << "port : " << m_port << "\n"
        << "filename : " << m_filename << "\n"
        << std::endl;
#endif
}


void HttpRequest::parse_ip_address(const std::string& domain)
{
    //if domai is website, the inet_addr will return INADDR_NONE
    unsigned long s_addr = inet_addr(domain.c_str());
    if(s_addr == INADDR_NONE)
    {
        char buffer[HTTP_DEFAULT_BUFFER];
        bzero(buffer, HTTP_DEFAULT_BUFFER);

        int herrno;
        struct hostent hostinfo, *hostinfo_ptr;

        //gethostbyname_r is thread_safety
        if(gethostbyname_r(domain.c_str(), &hostinfo, buffer, sizeof(buffer), &hostinfo_ptr, &herrno) < 0)
        {
            perror("ParseDomain : gethostbyname_r error...");
            m_ip_address = "";
            return;
        }
        else
        {
            //struct hostent::h_addr is the first ip_address, type is char *, which is network_byte_order
            //need to convert it to struct in_addr* and then use inet_ntoa(struct in_addr) to convert it to local_byte_order string
            m_ip_address = inet_ntoa(*(struct in_addr*)hostinfo_ptr->h_addr);
        }
    }
    else
    {
        m_ip_address = domain;
    }
}


std::string HttpRequest::get_filename() const
{
    return m_filename;
}

std::string HttpRequest::get_ip_address() const
{
    return m_ip_address;
}


int HttpRequest::get_port() const
{
    return m_port;
}


std::string HttpRequest::get_http_download_request() const
{
    //genterate request, why protocol is HTTP, why the protocol is HTTP?
    //when the url's protocol is HTTPS, the request is error if use HTTPS here but use HTTP is ok
    std::stringstream oss;
    oss << "GET " << m_url << " " << "HTTP" << "/1.1\r\n"
        << "HOST: " << m_ip_address << "\r\n"
        << "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.79 Safari/537.36\r\n"
        << "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n"
        << "Connection: Keep-Alive\r\n\r\n";
   return oss.str(); 
}


std::string HttpRequest::get_http_upload_request(const std::string& filename)
{
    long long int filesize = 0;
    struct stat fileinfo;
    if(lstat(filename.c_str(), &fileinfo) >= 0)
       filesize = fileinfo.st_size;

    std::stringstream oss;
    oss << "POST " << m_url << " HTTP/1.1\r\n"
        << "Host: " << m_ip_address << "\r\n"
        << "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.79 Safari/537.36\r\n"
        << "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n"
        << "Connection: Keep-Alive\r\n"
        << "Content-Type: multipart/form-data; " << "---------------------------boundary_linux_chrmoe_roc_\r\n"
        << "Content-Length: " << filesize << "\r\n"
        << "\r\n";
   
    return oss.str();   
}

std::string HttpRequest::get_http_upload_header(const std::string& filename)
{
    std::stringstream oss;
    oss << "-----------------------------boundary_linux_chrmoe_roc_\r\n"
        << "Content-Disposition: form-data; name=\"file\"; filename=\"" << filename << "\"\r\n"
        << "Content-Type: application/octet-stream\r\n"
        << "\r\n"; 
   return oss.str();
}

std::string HttpRequest::get_http_upload_tail()
{
    return "-----------------------------boundary_linux_chrmoe_roc_--\r\n";
}



////////////////////////////server//////////////////////////////
std::string HttpRequest::get_method() const
{
    return m_method;
}

std::string HttpRequest::get_argument() const
{
    return m_argument;
}

long long int HttpRequest::get_content_length() const
{
    return m_content_length;
}

bool HttpRequest::is_upload() const
{
    return m_upload;
}

void HttpRequest::parse_request(const std::string& request)
{
    parse_method(request);
    parse_filename_argument(request);
}

void HttpRequest::parse_method(const std::string& request)
{
    std::string::size_type front_idx = 0;
    std::string::size_type back_idx = request.find(" ");
    m_method = request.substr(front_idx, back_idx - front_idx);
}

void HttpRequest::parse_content_length(const std::string& request)
{
    std::string::size_type front_idx = request.find("Content-Length:") + 15;
    while(request[front_idx] == ' ')
        ++front_idx;
    std::string::size_type back_idx = request.find("\r\n", front_idx);
    std::string filesize_string = request.substr(front_idx, back_idx - front_idx);
    sscanf(filesize_string.c_str(), "%lld", &m_content_length);
}

void HttpRequest::parse_filename_argument(const std::string& request)
{
    if(m_method == "GET")
    {
        parse_get_filename(request);
    }
    else if(m_method == "POST")
    {
        parse_content_length(request);
        parse_post_filename_argument(request);
    }
}

void HttpRequest::parse_get_filename(const std::string& request)
{
    std::string::size_type front_idx = request.find(" ") + 1;
    std::string::size_type back_idx = request.find(" ", front_idx);
    std::string url = request.substr(front_idx, back_idx - front_idx);
    parse_url(url);
}

void HttpRequest::parse_post_filename_argument(const std::string& request)
{
    std::string::size_type front_idx = request.find("filename=") + 10;
    std::string::size_type back_idx = request.find("\"", front_idx);
    m_filename = request.substr(front_idx, back_idx - front_idx);
    front_idx = m_filename.find_last_of("/");
    m_filename = m_filename.substr(front_idx + 1);

    std::string::size_type content_type_idx = request.find("Content-Type: multipart/form-data");
    if(content_type_idx != std::string::npos)
    {
        m_upload = true;
    }
    else
    {
        m_upload = false;
        std::string::size_type front_idx = request.find("\r\n\r\n") + 4;
        std::string::size_type back_idx = request.find("\r\n", front_idx);
        m_argument = request.find(front_idx, back_idx - front_idx);
    }
}


