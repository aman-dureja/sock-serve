// SockServer.cpp

#include <sstream>

#include "SockServer.hpp"
#include "Response.hpp"

#define BACKLOG 10

SockServer::SockServer() : get_request_("GET"), responder_(new Response()) {}

int SockServer::initSocket(std::string const& port)
{
  int yes = 1;
  int rv;
  struct addrinfo hints, *servinfo;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &servinfo)) != 0) {
    std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
    return 1;
  }

  // loop through all the results and bind to the first we can
  for(binding_info_ = servinfo; binding_info_ != NULL; binding_info_ = binding_info_->ai_next) {
    if ((main_sockfd_ = socket(binding_info_->ai_family, binding_info_->ai_socktype, binding_info_->ai_protocol)) == -1) {
      std::perror("server error: socket");
      continue;
    }

    if (setsockopt(main_sockfd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      std::perror("server error: setsockopt");
      exit(1);
    }

    if (bind(main_sockfd_, binding_info_->ai_addr, binding_info_->ai_addrlen) == -1) {
      close(main_sockfd_);
      std::perror("server error: bind");
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo); // no longer need this struct

  return 0;
}

void sigchld_handler(int s)
{
  // waitpid() might overwrite errno
  int saved_errno = errno;
  while (waitpid(-1, NULL, WNOHANG) > 0);
  errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
      return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int SockServer::runServer()
{
  char s[INET6_ADDRSTRLEN];
  struct sockaddr_storage their_addr; // connector's address information
  struct sigaction sa;
  socklen_t sin_size;

  if (binding_info_ == nullptr)  {
    std::cerr << "server: failed to bind" << std::endl;
    exit(1);
  }

  if (listen(main_sockfd_, BACKLOG) == -1) {
    std::perror("server error: listen");
    exit(1);
  }

  sa.sa_handler = sigchld_handler; // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    std::perror("server error: sigaction");
    exit(1);
  }

  std::cout << "server: waiting for connections..." << std::endl;

  for(;;) {  // main accept() loop
    sin_size = sizeof their_addr;
    child_sockfd_ = accept(main_sockfd_, (struct sockaddr *)&their_addr, &sin_size);

    if (child_sockfd_ == -1) {
      std::perror("error: accept");
      continue;
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);

    std::cout << "server: got connection from " << s << std::endl;

    const unsigned int MBL = 4096;
    std::vector<char> buff(MBL);
    std::string rcv;
    int br = 0;

    if (!fork()) { // this is the child process
      close(main_sockfd_); // child doesn't need the listener

      do {
        br = recv(child_sockfd_, buff.data(), buff.size(), 0);
        if (br == -1) {
          std::cout << "ERROR!" << std::endl;
        }
        else {
          rcv.append(buff.cbegin(), buff.cend());
        }
      } while (br == MBL);

      std::cout << "~~~~~~~~~~RECEIVED THE FOLLOWING:~~~~~~~~~~ " << rcv << std::endl;

      if (rcv.length() > 0) {
        std::string http_method, http_path;
        std::stringstream requestStringStream(rcv);
        requestStringStream >> http_method >> http_path;
        std::cout << "Sending a response!" << std::endl;
        http_requests_map_[get_request_][http_path](child_sockfd_, *responder_);
      }

      close(child_sockfd_);
      exit(0);
    }

    close(child_sockfd_);
  }

  return 0;
}

void SockServer::httpGet(std::string const& req_path, responseFunction res_callback) {
  http_requests_map_[get_request_][req_path] = res_callback;
  return;
}
