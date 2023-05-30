#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <chrono>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <limits>
#include <utility>
#include "tcp_connect.h"

TcpConnect::TcpConnect(std::string ip, int port, std::chrono::milliseconds connectTimeout,
                       std::chrono::milliseconds readTimeout) :
        ip_(std::move(ip)), port_(port), connectTimeout_(connectTimeout), readTimeout_(readTimeout), sockfd_() {}

TcpConnect::~TcpConnect() {
    if (status != 2) {
        close(sockfd_);
    }
}

void TcpConnect::EstablishConnection() {
    struct pollfd pfd;
    int ret;
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip_.c_str());
    address.sin_port = htons(port_);
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) {
        throw std::runtime_error("Failed to create socket");
    }
    int flags = fcntl(sockfd_, F_GETFL, 0);
    fcntl(sockfd_, F_SETFL, flags | O_NONBLOCK);
    ret = connect(sockfd_, (struct sockaddr *) &address, sizeof(address));
    if (ret == 0) {
        // immediately
        flags = fcntl(sockfd_, F_GETFL, 0);
        fcntl(sockfd_, F_SETFL, flags & ~O_NONBLOCK);
        return;
    }
    pfd.fd = sockfd_;
    pfd.events = POLLOUT;
    ret = poll(&pfd, 1, connectTimeout_.count());
    if (ret == 0) {
        throw std::runtime_error("Connection timed out");
    } else if (ret < 0) {
        throw std::runtime_error("Error while connecting to remote host");
    } else {
        flags = fcntl(sockfd_, F_GETFL, 0);
        fcntl(sockfd_, F_SETFL, flags & ~O_NONBLOCK);
        return;
    }
}

void TcpConnect::SendData(const std::string &data) const {
    const char *buf = data.c_str();
    int len = data.length();
    while (len > 0) {
        int sent = send(sockfd_, buf, len, 0);
        if (sent < 0) {
            throw std::runtime_error("Failed to send data");
        }
        buf += sent;
        len -= sent;
    }
}


std::string TcpConnect::ReceiveData(size_t bufferSize) const {
    struct pollfd pfd;
    pfd.fd = sockfd_;
    pfd.events = POLLIN;

    size_t len = 4;
    if (bufferSize > 0) {
        len = bufferSize;
    } else {
//        {
//            int flags = fcntl(sockfd_, F_GETFL, 0);
//            if (flags == -1) throw std::runtime_error("Error in flag setting");
//            if ((flags & O_NONBLOCK) == 0) { flags |= O_NONBLOCK; }
//            else { flags &= ~O_NONBLOCK; }
//            if (fcntl(sockfd_, F_SETFL, flags) < 0) throw std::runtime_error("Error in flag setting#2");
//        }
        std::string lenbuf(4, 0);
        char *lenbufptr = &lenbuf[0];
//        {
//            int flags = fcntl(sockfd_, F_GETFL, 0);
//            if (flags == -1) throw std::runtime_error("Error in flag setting");
//            if ((flags & O_NONBLOCK) == 0) { flags |= O_NONBLOCK; }
//            else { flags &= ~O_NONBLOCK; }
//            if (fcntl(sockfd_, F_SETFL, flags) < 0) throw std::runtime_error("Error in flag setting#2");
//        }
        int ret = poll(&pfd, 1, readTimeout_.count());
        if (ret == 0) {
            throw std::runtime_error("Connection timed out");
        } else if (ret < 0) {
            throw std::runtime_error("Error while connecting to remote host");
        } else {
            int n = recv(sockfd_, lenbufptr, len, 0);
            if (n < 0) {
                throw std::runtime_error("Failed to receive data");
            }
            if (n < 4) throw std::runtime_error("Message length less then 4 bytes");
        }
        len = BytesToInt(lenbuf);
    }
    std::string data(len, 0);
    char *buf = &data[0];


    while (len > 0) {
        int ret = poll(&pfd, 1, readTimeout_.count());
        if (ret == 0) {
            throw std::runtime_error("Connection timed out");
        } else if (ret < 0) {
            throw std::runtime_error("Error while connecting to remote host");
        } else {
            int n = recv(sockfd_, buf, len, 0);
            if (n == 0) {
                throw std::runtime_error("Connection closed");
            }
            if (n < 0) {
                throw std::runtime_error("Failed to receive data");
            }
            buf += n;
            len -= n;
        }
    }
    return data;
}

void TcpConnect::CloseConnection() {
    close(sockfd_);
    status = 2;
}

const std::string &TcpConnect::GetIp() const {
    return ip_;
}

int TcpConnect::GetPort() const {
    return port_;
}