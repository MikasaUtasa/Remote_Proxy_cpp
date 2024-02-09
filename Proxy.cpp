//
// Created by eruan on 07.02.2024.
//

#include "Proxy.h"
#include <string>
#include <sys/socket.h>
#include <strings.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <unistd.h>
#include <cassert>
#include <arpa/inet.h>
#include <thread>
#include <string.h>



Proxy::Proxy(std::string srv_ip_addr ,int port) {
    Proxy::logging_file = "logs.txt";
    Proxy::listening_port = port;
    Proxy::server_port = 8080;
    Proxy::srv_ip_address = srv_ip_addr;
    startSerwer();
}

Proxy::Proxy(const Proxy &) {
    //assert(false);
}

Proxy& Proxy::operator = (const Proxy&) {
    //assert(false);
    //std::cout << "qwerrty" <<std::endl;
    return *this;
}


Proxy::~Proxy() {
    stopSerwer();
}


void Proxy::log(const std::string &message, int type) {
    time_t tmNow;
    tmNow = time(NULL);
    struct tm t = *localtime(&tmNow);
    log_file << t.tm_hour << ":" << t.tm_min << ":" << t.tm_sec << "  ";

    switch (type) {
        case 0:
            log_file << "INFO[0] ";
            std::cout << "INFO[0] ";
            break;
        case 1:
            log_file << "ERR[1] ";
            std::cout << "ERR[1] ";
            break;
    }
    log_file << message << std::endl;
    std::cout << message << std::endl;
}



int Proxy::startSerwer() {
    bzero((char *) &proxy_addr, sizeof(proxy_addr)); //Zerowanie buforu
    Proxy::log_file.open(Proxy::logging_file);
    std::ostringstream ss;
    ss << "Serwer start Logging" << std::endl;
    Proxy::log(ss.str(), 0);


    cli_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    srv_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //setsockopt(sockfd ,SO_REUSEADDR, 1);
    if (cli_sockfd < 0) //Nieudane zgniazdo zwraca -1
        log("ERROR opening client socket", 1);
    if (srv_sockfd < 0) //Nieudane zgniazdo zwraca -1
        log("ERROR opening server socket", 1);

    Proxy::proxy_addr.sin_family = AF_INET;
    Proxy::proxy_addr.sin_port = htons(listening_port);
    Proxy::proxy_addr.sin_addr.s_addr = INADDR_ANY;

    Proxy::serv_addr.sin_family = AF_INET;
    Proxy::serv_addr.sin_port = htons(server_port);
    Proxy::serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(srv_sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        log("ERROR connecting with server", 1);



    if (bind(cli_sockfd, (sockaddr*) &proxy_addr, sizeof(proxy_addr)) < 0) {
        log("Bind ERROR", 1);
        exit(-1);
        return 1;
    } //Nieudany bind zwraca -1

    //TCPserwer::startListen();
    return 0;
}

void Proxy::stopSerwer() {
    close(cli_sockfd);
    //close(newsockfd);
    Proxy::log_file.close();
    exit(0);
}



void Proxy::startListen() {
    if (listen(cli_sockfd, 5) < 0) {
        log("Socket listen failed", 1);
    }
    std::ostringstream ss;
    ss << "*** Listening on address: "
       << inet_ntoa(proxy_addr.sin_addr)
       << " PORT: " << ntohs(proxy_addr.sin_port)
       << " ***\n";
    log(ss.str(), 0);
    int serverSocket = srv_sockfd;
    sockaddr_in serverAddress = Proxy::serv_addr;

    std::thread serverThread([this, serverSocket, serverAddress]() {
        sockaddr_in copyOfServerAddress = serverAddress;
        this->handleServer(serverSocket, copyOfServerAddress);
    });
    Threads.emplace_back(std::move(serverThread));

    while (true) {
        sockaddr_in clientAddress;
        //sockaddr_in serverAddress = Proxy::serv_addr;
        socklen_t clientAddressSize = sizeof(clientAddress);
        //int serverSocket = srv_sockfd;
        int clientSocket = accept(cli_sockfd, (sockaddr*)&clientAddress, &clientAddressSize);


        if (clientSocket == -1) {
            ss << "Failed to accept connection from "
               << inet_ntoa(clientAddress.sin_addr)
               << ":"
               << ntohs(clientAddress.sin_port);
            log(ss.str(), 1);
        }

        ss.clear();
        ss << "Connection from "
           << inet_ntoa(clientAddress.sin_addr)
           << ":"
           << ntohs(clientAddress.sin_port);
        log(ss.str(), 0);
        ss.str("");
        //Proxy::handleConnection(clientSocket, srv_sockfd, cli_addr, serv_addr);
        //Proxy::handleClient(clientSocket, clientAddress);
        //Proxy::handleServer(srv_sockfd, serv_addr);

        std::thread clientThread([this, clientSocket, clientAddress]() {
            sockaddr_in copyOfClientAddress = clientAddress;
            this->handleClient(clientSocket, copyOfClientAddress);
        });
        /*
        std::thread serverThread([this, serverSocket, serverAddress]() {
            sockaddr_in copyOfServerAddress = serverAddress;
            this->handleServer(serverSocket, copyOfServerAddress);
        });*/

        Threads.emplace_back(std::move(clientThread));
        //Threads.emplace_back(std::move(serverThread));
    }

}



void Proxy::handleClient(/*void *new_sockfd*/int new_sock, sockaddr_in &client) {
    std::cout << "HelloCLI" << std::endl;

    while (new_sock >= 0) {

        char buffer[BUFFER_SIZE] = {0};
        std::ostringstream ss;
        std::ostringstream client_str;
        client_str << inet_ntoa(client.sin_addr) << ":" << ntohs(client.sin_port);
        std::string buff;// = "Hello from serwer";
        //sendData(buff);
        int rcv = read(new_sock, buffer, BUFFER_SIZE);
        std::cout << buffer << std::endl;
        if (rcv <= 0) {
            log("Failed to read bytes from peer socket connection", 0);
            close(new_sock);
            break;
        }
        ss << "------ Received data from " << client_str.str() << "   :" << buffer;
        log(ss.str(), 0);
        ss.str("");
        ss << buffer;
        std::string message = ss.str();


        sendData(message, srv_sockfd);


        /*int n = write(cli_sockfd,buffer,strlen(buffer));
        if (n < 0)
            log("ERROR writing to socket", 0);

        bzero(buffer,256);*/



    }
}

void Proxy::handleServer(int new_sock, sockaddr_in &client) {
    std::cout << "HelloSRV" << std::endl;

    std::ostringstream ss;
    std::ostringstream client_str;
    client_str << inet_ntoa(client.sin_addr) << ":" << ntohs(client.sin_port);
    std::string buff = "Hello from proxy";
    sendData(buff, srv_sockfd);
    char buffer[BUFFER_SIZE] = {0};

    while (new_sock >= 0) {



    int rcv = read(new_sock, buffer, BUFFER_SIZE);
    std::cout << buffer << std::endl;
    if (rcv <= 0) {
        log("Failed to read bytes from client socket connection", 0);
        close(new_sock);
        break;
    }
        ss << "------ Received data from " << client_str.str() << "   :" << buffer;
        log(ss.str(), 0);

        ss.str("");
        ss << buffer;
        std::string message = ss.str();

        if(cli_sockfd > 1) {
            sendData(message, cli_sockfd);

        }
        memset(buffer, 0, sizeof buffer);


        //sendData(message, cli_sockfd);

        /*int n = write(cli_sockfd,buffer,strlen(buffer));
        if (n < 0)
            log("ERROR writing to socket", 0);

        bzero(buffer,256);*/



    }

}


void Proxy::sendData(std::string &buffer, int &socket) {
    long bytesSent;
    std::ostringstream ss;
    bytesSent = write(socket, buffer.c_str(), buffer.size());
    if (bytesSent == buffer.size())
    {
        ss << "------ Proxy Response sent to peer:   " << buffer;
        log(ss.str(), 0);
        //log("------ Server Response sent to client:");
    }
    else
    {
        log("Error sending response to peer", 1);
    }
}