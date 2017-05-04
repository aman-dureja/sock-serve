// Response.h

#include <string>

#ifndef RESPONSE_H
#define RESPONSE_H

class Response
{
  private:
    static const std::string http_version_;
  public:
    Response();
    ~Response();
    void sendString(int const& socketfd, std::string const& msg);
    void sendFile(int const& socketfd, std::string const& file_type, std::string const& file_path);
};

#endif
