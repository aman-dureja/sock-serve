#include "SockServer.hpp"

void testFunc(int socketfd, Response res) {
  std::cout << "~~~~~~testFunc~~~~~~" << std::endl;
  res.sendString(socketfd, "SockServer is functional!");
  return;
}

int main()
{
  SockServer *test = new SockServer();
  test->initSocket("3000");
  test->httpGet("/", testFunc);
  test->runServer();
  return 0;
}
