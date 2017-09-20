#include "http_client.h"


int main(int argc, char *argv[])
{
    if(argc != 3 && argc != 4)
    {
        std::cerr << "Usage: argument is incorrent...";
        std::cerr << "Usage: d for download file with url\n     u for upload file with url and filename\n";
        return -1;
    }
    //const std::string url = "https://nodejs.org/dist/v4.2.3/node-v4.2.3-linux-x64.tar.gz";
    HttpClient http_client;
    if(strcmp(argv[1], "d") == 0)
        http_client.download_file(argv[2]);
    else if(strcmp(argv[1], "u") == 0)
        http_client.upload_file(argv[2], argv[3]);
    return 0;
}
