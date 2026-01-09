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



std::string exec_cgi() {
    int pipefd[2];
    // 1. Create a Pipe
    // pipefd[0] is for READING (Parent uses this)
    // pipefd[1] is for WRITING (Child uses this)
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return "Internal Server Error";
    }

    // 2. Fork the Process
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return "Internal Server Error";
    }

    if (pid == 0) {
        // --- CHILD PROCESS (Will become Python) ---
        
        // A. Close the READ end (Child doesn't read from itself)
        close(pipefd[0]);

        // B. Redirect STDOUT to the WRITE end of the pipe
        // Now, anything Python prints goes into the pipe, not the screen.
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]); // Close original FD after duplication

        // C. Execute the Script
        // execve requires C-style arrays of char*
        char* script = (char*)"./time_server.py";
        char* interpreter = (char*)"/usr/bin/python3";
        char* argv[] = {interpreter, script, NULL}; // Arguments: [Program, Script, NULL]
        char* envp[] = {NULL}; // Environment variables

        // execve REPLACES the current process with Python.
        // If successful, this code effectively STOPS running here.
        if (execve(interpreter, argv, envp) == -1) {
            perror("execve");
            exit(1); // Kill child if exec fails
        }
    } 
    else {
        // --- PARENT PROCESS (The C++ Server) ---
        
        // A. Close the WRITE end (Parent only reads)
        // CRITICAL: If you don't close this, read() will hang forever waiting for EOF!
        close(pipefd[1]); 

        // B. Read the output from the Child
        std::string result;
        char buffer[128];
        ssize_t bytesRead;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0'; // Null-terminate
            result += buffer;
        }
        close(pipefd[0]);

        // C. Wait for the Child to die (Reaping the Zombie)
        waitpid(pid, nullptr, 0); 
        
        return result;
    }

    return ""; // Should not reach here
}


// a helpter to read a file into a string 
std::string load_file(const std::string& filename){
  std::ifstream file(filename);
  if (!file.is_open()){
    return "";
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}



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
  address.sin_port = htons(8080); // host ot network short (converts endianness)


  // links the "server_fd" to the address config 
  if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0){
    std::cerr << "Bind failed. Is port 8080 busy?\n";
    return 1;
  }

  // Listen 
  // 10 is the "Backlog" (how many pending connections the OS waits) to accept before it starts rejecting new ones 
  if (listen(server_fd, 10) < 0){
    // listen() tells the kernel to start accepting SYN packets.
    std::cerr << "Listen failed";
    return 1;
  }

  std::cout << "Server listening on port 8080 ... waiting for connections. \n";
  std::cout << "Ctrl + Click on: http://localhost:8080\n";

  // The Kernel is now handling "SYN/SYN-ACK/ACK" automatically in the background.

  while (true){
    // accepts a conneciton
    // this call blocks (i.e., the program stops here until someone connects)
    int addrlen = sizeof(address);
    int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (new_socket < 0){
      std::cerr << "Accept failed\n";
      // std::cout << "Accept failed but still continue ...\n";
      continue;
      // return 1;
    }
  
    // 1. reads the request 
    char buffer[30000] = {0};
    read(new_socket, buffer, 30000); // reads up to 30000 bytes from the socket 

    // std::cout << "--------- RECEIVED RAW REQUEST ----------\n";
    // std::cout << buffer << std::endl;
    // std::cout << "-----------------------------------------\n";
  
    // 2. Parse the "method" and the "Path"
    std::istringstream request_stream(buffer);
    std::string method, path, version;


    // "GET / HTTP/1.1" -> method="GET", path="/", version="HTTP/1.1"
    request_stream >> method >> path >> version;

    std::cout << "Method: " << method <<  " | Path: " << path << "\n";
  
    // // send a basic reponse so the browser doesn't spin forever
    // const char* hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello World!";
    // write(new_socket, hello, 60);

    // 3. Router (determine what to send back)
    std::string response_body;
    std::string status_line;

    if (path == "/"){
      response_body = load_file("index.html");
      status_line = "HTTP/1.1 200 OK\r\n";
    } 
    else if (path == "/time"){
      response_body = exec_cgi();
      status_line = "HTTP/1.1 200 OK\r\n";
    } 
    else {
      // 404 Not Found error handling
      response_body = "<html><h1>404 Not Found</h1></html>";
      status_line = "HTTP/1.1 404 Not Found\r\n";
    }

    // 4. Construct the full response 
    std::string response = status_line +
                            "Content-Type: text/html\r\n" +
                            "Content-Length: " + std::to_string(response_body.length()) + "\r\n" +
                            "\r\n" + // The blank line separating Headers from Body
                            response_body;
    
    // 5. Send the data! or you will get an error (ERR_EMPTY_RESPONSE)
    write(new_socket, response.c_str(), response.length());
    
    // close the socket (cleanup)
    close(new_socket);
  }


  close(server_fd);



  return 0;
}
