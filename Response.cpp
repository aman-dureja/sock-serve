// Response.cpp

#include <sstream>
#include <fstream>

#include <netinet/in.h>

#include "Response.hpp"

const std::string Response::http_version_ = "1.1";

Response::Response() {}

Response::~Response() {}

void Response::sendString(int const& socketfd, std::string const& msg)
{
  std::stringstream response_string_stream;
  response_string_stream << http_version_ << " 200 OK\r\nContent-Type: text/plain\r\n\r\n" << msg << std::flush;
  std::string string_to_send = response_string_stream.str();
  size_t bytes_sent = 0;
  size_t length_remaining = string_to_send.length();
  const char *buffer = string_to_send.c_str();

  while (length_remaining > 0) {
    bytes_sent = send(socketfd, buffer, length_remaining, 0);
    if (bytes_sent == -1) {
      std::perror("response error: send string");
    }
    else if (bytes_sent == 0) {
      break;
    }
    buffer += bytes_sent;
    length_remaining -= bytes_sent;
  }

  return;
}

void Response::sendFile(int const& socketfd, std::string const& file_type, std::string const& file_path)
{
  char *file_buffer;
  size_t file_size, bytes_sent, length_remaining;
  std::ifstream file(file_path, std::ios::in|std::ios::binary|std::ios::ate);
  std::stringstream response_string_stream;

  file_size = file.tellg();
  file.seekg(0, std::ios::beg);

  file_buffer = new char[file_size];
  file.read(file_buffer, file_size);
  file.close();

  response_string_stream << http_version_ << " 200 OK\r\nContent-Type: " << file_type << "\r\n\r\n" << std::flush;

  const char *p = response_string_stream.str().c_str();
  int length = response_string_stream.str().length();
  int n;

  while (length > 0) {
    n = send(socketfd, p, length, 0);
    p += n;
    length -= n;
  }

  length_remaining = file_size;

  while (file_size > 0) {
    bytes_sent = send(socketfd, file_buffer, length_remaining, 0);
    if (bytes_sent == -1) {
      std::perror("response error: send file");
    }
    else if (bytes_sent == 0) {
      break;
    }
    file_buffer += bytes_sent;
    length_remaining -= bytes_sent;
  }

  return;
}
