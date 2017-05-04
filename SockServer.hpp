// SockServer.h

#include <unordered_map>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <csignal>

#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "Response.hpp"

#ifndef SOCKSERVER_H
#define SOCKSERVER_H

class SockServer
{
  private:
    int main_sockfd_, child_sockfd_;
    Response *responder_;
    typedef void (*responseFunction)(int, Response);
    struct addrinfo *binding_info_;
    const std::string get_request_;
    std::unordered_map< std::string, std::unordered_map<std::string, responseFunction> > http_requests_map_;
  public:
    SockServer();
    ~SockServer();
    int initSocket(std::string const& port);
    int serveStream();
    int runServer();
    void httpGet(std::string const& req_path, responseFunction res_callback);
};

#endif
