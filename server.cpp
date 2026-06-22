#include <iostream>
#include <istream>
#include <sys/socket.h>
#include <netinet/in.h> // for sockaddr_in 
#include <unistd.h>


#include <fstream> // for reading files 
#include <sstream> // for parsing strings 
#include <string>


#include <sys/wait.h> // for waitpid()
#include <vector>



#include <sys/epoll.h> // the linux event poll api 
#include <fcntl.h> // to set non-blocking flags 


// #include "ThreadPool.h" 
// #include "HttpParser.h"
// #include "ServerStats.h"

#include <sys/stat.h>




#include <mutex>




int main(){
  // create the socker 
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (server_fd < 0){
    std::cerr << "Failed to create socket\n";
    return 1;
  }


  // to fix "Address alr in use" error when we restart the server too quickly and the OS hasn't cleaned up the port 
  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));


  // bind the socket to and IP and port 
  struct sockaddr_in address; 
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY; // listens on 0.0.0.0 (all interfaces)

  int PORT = 8080;
  address.sin_port = htons(PORT); // host ot network short (converts endianness)


  // links the "server_fd" to the address config 
  if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0){
    std::cerr << "Bind failed. Is port " << PORT <<  " busy?\n";
    std::cerr << "Make sure that it is not busy. Exiting the program ...";
    return 1;
  } 



  // Listen 
  if (listen(server_fd, 10) < 0){
    // listen() tells the kernel to start accepting SYN packets.
    std::cerr << "Listen failed";
    return 1;
  }


  std::cout << "Event Loop + Threadpool started\n."
            << "\nServer listening on port " 
            << PORT 
            << " ... waiting for connections. \n";
  
  
  
  std::cout << "Ctrl + Click on: http://localhost:" << PORT << "\n";
  // The Kernel is now handling "SYN/SYN-ACK/ACK" automatically in the background.


  while (true) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            
    if (client_fd < 0){
        throw std::runtime_error("accept failed");
    }

    // handle_client(client_fd);
}


  close(server_fd);



  return 0;
}
