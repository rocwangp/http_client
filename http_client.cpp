#include "http_client.h"


HttpClient::HttpClient() :
    m_socket(),
    m_http_request(),
    m_http_response()
{

}


HttpClient::~HttpClient()
{
  
}


void HttpClient::download_file(const std::string& url)
{
    m_socket.CreateSocket();

    //when user input website(url), client(brower) first parse the url to obtain
    //protocol : HTTP / HTTPS / etc
    //domain : to obtain ip address
    //port : if there isn't port, default is 8080
    //filename : the file to access
    //
    //then client(brower) start generate request and then send to server
    m_http_request.parse_url(url);
    std::string request_header = m_http_request.get_http_download_request();
    if(request_header == "")
        return;

    //client(brower) try to connect to server in order to send request
    if(m_socket.Connect(m_http_request.get_ip_address(), m_http_request.get_port()) == false)
    {
        std::cerr << "connect to server error...\n";
        m_socket.CloseSocket();
        return;
    }
   
    //client(brower) send request to server
    if(m_socket.Send(request_header) <= 0)
    {
        std::cerr << "send request error...\n";
        m_socket.CloseSocket();
        return;
    }

    //when server receive request from client(brower), server start to parse the request
    //and judge whether it is a valid request , include
    //parse the method : GET / POST / etc
    //find the file client access : 
    //   when method is GET, the filename is after the GET
    //   when method is POST, the filename is in the last line
    //when judge the request is valid, the server send a response to client(brower)
    std::string response;
    if(m_socket.Recv(response) <= 0)
    {
        std::cerr << "receive response error...\n";
        m_socket.CloseSocket();
        return;
    }
    m_http_response.parse_response(response);

    
    //the response include the status code, like 200(successful), 404(error), 302(redirection) and etc
    //when the code is 302, show that the url client access redirect to a new url
    //so need to parse response to judge the code and get the redirection url if needed
    //
    //when redirection, carry out download job again with a new url
    int status_code = m_http_response.get_status_code();
    if(status_code == HTTP_STATUS_REDIR)
    {
        std::cerr << "redirection a new url to download" << std::endl;
        m_socket.CloseSocket();
        this->download_file(m_http_response.get_location());
        return;
    }
    else if(status_code == HTTP_STATUS_BAD)
    {
        std::cerr << "resquest error : request_header is incorrent...";
        return;
    }
    else if(status_code == HTTP_STATUS_NFOUND)
    {
        std::cerr << "request error : the source not exist...";
    }
    else if(status_code == HTTP_STATUS_OK)
    {
        //open a new thread to treat the download job 
        //and the process wait other command 
        pthread_t tid;
        pthread_create(&tid, NULL, HttpClient::process_download, this);
        pthread_join(tid, NULL);
    }
    else if(status_code == HTTP_STATUS_OTHER)
    {
        std::cerr << "request error : unknown error...";
    }

    m_socket.CloseSocket();
}


void *HttpClient::process_download(void *arg)
{
    HttpClient *http_client = static_cast<HttpClient*>(arg);
    
    long long int filesize = http_client->m_http_response.get_content_length();
    std::string filename_with_dir = http_client->m_http_request.get_filename();
    std::string filename = filename_with_dir.substr(filename_with_dir.find_last_of('/') + 1);

    std::ofstream out;
    out.open(filename.c_str(), std::ios_base::binary | std::ios_base::out);
    if(!out.is_open())
    {
        std::cerr << "create or open file : " << filename << " error...\n";
        pthread_exit(NULL);
        return NULL;
    }
   
    std::cout << "start download..." << std::endl;
    
    long long int downloaded_size = 0;
    std::string message;
    char buffer[DOWNLOAD_BUFFER_SIZE];
    bzero(buffer, sizeof(buffer));
    while(true)
    {
        //use socket's recv or io's read to read message from server
        //the message may contain many '\0' but all this '\0' need to write to file
        //so project need to use the return_value(represent the number of read_bytes)
        //and write file the same number bytes
        //
        //but if only use out.write(message.c_str(), message.size()), message.size() always less then ret
        //bacause the existance of more then one '\0', so need to use out.write(message.c_str(), ret)
        //in this case, project will write byte from the address of message.c_str()
        //and end to message_c_str() + ret
        //
        //this solution is same with upload file and server receive file 
        //

        int ret = recv(http_client->m_socket.GetSocket(), buffer, sizeof(buffer), 0);
      
        if(ret < 0)
        {
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            break;
        }
        else if(ret == 0)
        {
            break;
        }
        else
        {
            out.write(buffer, ret);
            downloaded_size += ret;
            HttpClient::show_process_bar(downloaded_size, filesize);
        }

        if(downloaded_size >= filesize)
            break;
    }
   
    out.close();
    std::cout << "download completed..." << std::endl;
    pthread_exit(NULL);
}


void HttpClient::show_process_bar(long long int current_size, long long int total_size)
{
    double precent = (double)current_size / total_size;
    const int num_total = 50;
    int num_show = (int)(num_total * precent);

    if(num_show == 0)
        num_show = 1;

    if(num_show > num_total)
        num_show = num_total;

    char sign[51] = {0};
    memset(sign, '=', num_total);
    
    //%-*.*s : -: result left align
    //         *.: the total bit wide
    //         .*: print bit wide
    printf("\r%.2f%%\t[%-*.*s] %.2f/%.2fMB", precent * 100, num_total, num_show, sign, current_size / 1024.0 / 1024.0, 
           total_size / 1024.0 / 1024.0);
    fflush(stdout);
    if(num_show == num_total)
        printf("\n");
}

long long int HttpClient::parse_filesize(const std::string& filename)
{
    struct stat fileinfo;
    if(lstat(filename.c_str(), &fileinfo) < 0)
        return 0;
    else
        return fileinfo.st_size;
}

void HttpClient::upload_file(const std::string& url, const std::string& filename)
{
    m_filename = filename;
    m_socket.CreateSocket();
    m_http_request.parse_url(url);
    
    if(m_socket.Connect(m_http_request.get_ip_address(), m_http_request.get_port()) == false)
    {
        std::cerr << "connect to server error..." << std::endl;
        m_socket.CloseSocket();
        return;
    }

    //upload file to web server need to send request header with special content
    //POST url HTTP/1.1
    //...
    //Content-Type: multipart/form-data; -------------------boundary*****\r\n
    //Content-Length: ***\r\n\r\n
    //
    //multipart/form-data represent this is a file upload protocol, and after it the "------*" is identifier
    //when appear it again, server will know the content after it is file_data, and need to save it to file
    std::string request_header = m_http_request.get_http_upload_request(m_filename);
    if(m_socket.Send(request_header) <= 0)
    {
        std::cerr << "send request error..." << std::endl;
        m_socket.CloseSocket();
        return;
    }

    pthread_t tid;
    pthread_create(&tid, NULL, HttpClient::process_upload, this);
    pthread_join(tid, NULL);
    m_socket.CloseSocket();
}


void *HttpClient::process_upload(void *arg)
{
    HttpClient* http_client = static_cast<HttpClient*>(arg);
    
    //find header with "---------****" identifier to tell server after this is data
    std::string message = http_client->m_http_request.get_http_upload_header(http_client->m_filename);
    if(http_client->m_socket.Send(message) <= 0)
    {
        std::cerr << "send message header error...\n";
        pthread_exit(NULL);
        return NULL;
    }

    std::ifstream in;
    in.open(http_client->m_filename.c_str(), std::ios_base::binary | std::ios_base::in);
    if(!in.is_open())
    {
        std::cerr << "open file error...\n";
        pthread_exit(NULL);
        return NULL;
    }

    long long int filesize = http_client->parse_filesize(http_client->m_filename);

    std::cout << "start upload..." << std::endl;   
    std::cout << "filename is " <<  http_client->m_filename 
              << " filesize is " << filesize << std::endl; 
  
    char buffer[UPLOAD_BUFFER_SIZE];
    bzero(buffer, sizeof(buffer));
    long long int  uploaded_size = 0;
  
    while(!in.eof())
    {
        in.read(buffer, UPLOAD_BUFFER_SIZE);
        int count = in.gcount();
        
        //here is alse use count and don't use strlen(buffer);
        send(http_client->m_socket.GetSocket(), buffer, count, MSG_NOSIGNAL);
        uploaded_size += count;
     
        HttpClient::show_process_bar(uploaded_size, filesize);
        bzero(buffer, HTTP_DEFAULT_BUFFER);
    }
    
    //when send file data over, send the tail with "--------*--";
    message = http_client->m_http_request.get_http_upload_tail();
    http_client->m_socket.Send(message);

    in.close();
    std::cout << "upload completed..." << std::endl;
    pthread_exit(NULL);
}
