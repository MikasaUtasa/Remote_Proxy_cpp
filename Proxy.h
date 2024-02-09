//
// Created by eruan on 07.02.2024.
//

#ifndef REMOTE_PROXY_PROXY_H
#define REMOTE_PROXY_PROXY_H

#include <cstdlib>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>


class Proxy {
public:
    Proxy(std::string srv_ip_addr ,int port);
    Proxy(const Proxy&);
    Proxy& operator = (const Proxy&);
    ~Proxy();
    void startListen();
    static const int BUFFER_SIZE = 30720;



private:
    std::string srv_ip_address;
    std::string logging_file;
    std::ofstream log_file;
    std::vector<std::thread> Threads;

    int listening_port, cli_sockfd, srv_sockfd, server_port;
    struct sockaddr_in serv_addr, cli_addr, proxy_addr;
    char buffer[256];

    void log(const std::string &message, int type);
    int startSerwer();
    void stopSerwer();
    void handleClient(/*void *new_sockfd*/ int new_sock, sockaddr_in &client);
    void handleServer(/*void *new_sockfd*/ int new_sock, sockaddr_in &server);
    void sendData(std::string  &buffer, int &socket);


};


#endif //REMOTE_PROXY_PROXY_H
