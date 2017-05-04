# sock-serve
An HTTP server implementation in C++, using sockets.

See `example.cpp` for a demonstration on basic usage of the API.

To use it, include the module in your code. Create a new `SockServer` object:

```c++
SockServer *myServer = new SockServer(); 
```

Initialize the server object with a string representation of the port you wish to run the server on:

```c++
myServer->initialize("8080");
```

Now, you can handle HTTP GET requests at certain routes by using callback functions:

```c++
void testCallback(int socketfd, Response res) {
  std::cout << "In the callback!" << std::endl;
  return;
}

main() {
  SockServer *myServer = new SockServer(); 
  myServer->initialize("8080");
  myServer->httpGet("/", testCallback);
}
```

In the above example, whenever a client navigates to the root path `/`, the server will log `"In the callback!"`.

All callback functions must take two parameters: `int socketfd` and `Response res`. The `socketfd` is the UNIX file descriptor of the socket and must be passed into methods that `res` calls. Retreiving it is done without any work on the client side.

The `Response` object can call methods to respond to the client with either a string or a file:

```c++
res.sendString(socketfd, "SockServer is functional!"); // takes the socket file descriptor and the string you want to send
res.sendFile(socketfd, "text/html", "./index.html"); // takes the socket file descriptor, a string describing the file type (like 'text/plain', 'text/html', 'application/json', etc.), and a string giving the path of the file on the local machine
```

To start the server, simply call the `runServer` function of your server object:

```c++
myServer->runServer();
```

The above code will start listening on the port you initialized the server with, and requests will be handled via callback functions.
