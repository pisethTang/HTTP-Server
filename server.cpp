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




// ======================================================================

#include <mutex>



// Global Instance (Simple for this demo)
// ServerStats global_stats;



// void set_nonblocking(int fd){
//   // force 
//   int flags = fcntl(fd, F_GETFL, 0);
//   fcntl(fd, F_SETFL, flags | O_NONBLOCK);
// }


// std::string exec_cgi() {
//     int pipefd[2];
//     // 1. Create a Pipe
//     // pipefd[0] is for READING (Parent uses this)
//     // pipefd[1] is for WRITING (Child uses this)
//     if (pipe(pipefd) == -1) {
//         perror("pipe");
//         return "Internal Server Error";
//     }

//     // 2. Fork the Process
//     pid_t pid = fork();

//     if (pid == -1) {
//         perror("fork");
//         return "Internal Server Error";
//     }

//     if (pid == 0) {
//         // --- CHILD PROCESS (Will become Python) ---
        
//         // A. Close the READ end (Child doesn't read from itself)
//         close(pipefd[0]);

//         // B. Redirect STDOUT to the WRITE end of the pipe
//         // Now, anything Python prints goes into the pipe, not the screen.
//         dup2(pipefd[1], STDOUT_FILENO);
//         close(pipefd[1]); // Close original FD after duplication

//         // C. Execute the Script
//         // execve requires C-style arrays of char*
//         char* script = (char*)"./time_server.py";
//         char* interpreter = (char*)"/usr/bin/python3";
//         char* argv[] = {interpreter, script, NULL}; // Arguments: [Program, Script, NULL]
//         char* envp[] = {NULL}; // Environment variables

//         // execve REPLACES the current process with Python.
//         // If successful, this code effectively STOPS running here.
//         if (execve(interpreter, argv, envp) == -1) {
//             perror("execve");
//             exit(1); // Kill child if exec fails
//         }
//     } 
//     else {
//         // --- PARENT PROCESS (The C++ Server) ---
        
//         // A. Close the WRITE end (Parent only reads)
//         // CRITICAL: If you don't close this, read() will hang forever waiting for EOF!
//         close(pipefd[1]); 

//         // B. Read the output from the Child
//         std::string result;
//         char buffer[128];
//         ssize_t bytesRead;
//         while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
//             buffer[bytesRead] = '\0'; // Null-terminate
//             result += buffer;
//         }
//         close(pipefd[0]);

//         // C. Wait for the Child to die (Reaping the Zombie)
//         waitpid(pid, nullptr, 0); 
        
//         return result;
//     }

//     return ""; // Should not reach here
// }


// // a helpter to read a file into a string 
// std::string load_file(const std::string& filename){
//   std::ifstream file(filename);
//   if (!file.is_open()){
//     return "";
//   }
//   std::stringstream buffer;
//   buffer << file.rdbuf();
//   return buffer.str();
// }


// std::string get_mime_type(const std::string& path) {
//     if (path.find(".html") != std::string::npos) return "text/html";
//     if (path.find(".css") != std::string::npos) return "text/css";
//     if (path.find(".js") != std::string::npos) return "application/javascript";
//     if (path.find(".png") != std::string::npos) return "image/png";
//     if (path.find(".jpg") != std::string::npos) return "image/jpeg";
//     if (path.find(".ico") != std::string::npos) return "image/x-icon";
//     return "text/plain";
// }



// // request handling
// void handle_client(int client_fd) {
//     global_stats.increment_requests();
//     char buffer[30000] = {0};
//     ssize_t bytes_read = read(client_fd, buffer, 30000);

//     if (bytes_read > 0) {
//         std::string raw_data(buffer, bytes_read);
//         HttpRequest req = HttpParser::parse(raw_data);
//         std::string status_line, response_headers, response_body;
        
//         // --- 1. COOKIE LOGIC ---
//         // Check if user has a cookie. If not, set one.
//         bool is_new_user = true;
//         std::string set_cookie_header = "";
        
//         if (req.headers.count("Cookie") && req.headers["Cookies"].find("session_id=") != std::string::npos) {
//             // In a real app, we would check if the session ID is valid
//             is_new_user = false;
//         } else {
//             set_cookie_header = "Set-Cookie: session_id=user_12345; Path=/; HttpOnly\r\n";
//         }

//         // --- 2. ROUTING ---
        
//         // API: Dashboard Stats
//         if (req.method == "GET" && req.path == "/stats") {
//             response_body = global_stats.to_json();
//             status_line = "HTTP/1.1 200 OK\r\n";
//             response_headers = "Content-Type: application/json\r\n";
//         }
//         // API: Trigger Stress Test
//         else if (req.method == "GET" && req.path.find("/trigger-stress") == 0) {
//             // Parse query parameter: /trigger-stress?count=500
//             int count = 100; // Default
//             size_t count_pos = req.path.find("count=");
//             if (count_pos != std::string::npos) {
//                 count = std::stoi(req.path.substr(count_pos + 6));
//             }
            
//             // Build shell command: seq N | xargs -P 100 curl
//             std::string cmd = "seq " + std::to_string(count) + 
//                             " | xargs -P 100 -I {} curl -s -o /dev/null http://127.0.0.1:8080/ &";
            
//             // Execute in background
//             system(cmd.c_str());
            
//             response_body = "{\"status\":\"started\",\"count\":" + std::to_string(count) + "}";
//             status_line = "HTTP/1.1 200 OK\r\n";
//             response_headers = "Content-Type: application/json\r\n"
//                              "Access-Control-Allow-Origin: *\r\n";
//         }
//         // API: File Upload (POST)
//         else if (req.method == "POST" && req.path == "/upload") {
//             std::ofstream out_file("uploaded_data.txt");
//             out_file << req.body;
//             out_file.close();
//             response_body = "File Uploaded. Size: " + std::to_string(req.body.length()) + "\n. Check your uploaded_data.txt";
//             status_line = "HTTP/1.1 201 Created\r\n";
//             response_headers = "Content-Type: text/plain\r\n";
//         }
//         // API: DELETE File
//         else if (req.method == "DELETE") {
//             // SECURITY WARNING: In prod, you must sanitize this path to prevent deleting /etc/passwd!
//             // For now, we assume the user provides a local filename like "/uploaded_data.txt"
//             std::string filename = "." + req.path; // "/file.txt" -> "./file.txt"
            
//             if (remove(filename.c_str()) == 0) {
//                 response_body = "File deleted successfully";
//                 status_line = "HTTP/1.1 200 OK\r\n";
//             } else {
//                 response_body = "File not found or delete failed";
//                 status_line = "HTTP/1.1 404 Not Found\r\n";
//             }
//             response_headers = "Content-Type: text/plain\r\n";
//         }
//         // STATIC FILES (The "Catch-All" for GET)
//         else if (req.method == "GET") {
//             // Check for CGI route FIRST (before file checks)
//             if (req.path == "/time") {
//                 response_body = exec_cgi();
//                 status_line = "HTTP/1.1 200 OK\r\n";
//                 response_headers = "Content-Type: text/html\r\n";
//             }
//             else {
//                 std::string filepath = "." + req.path;
                
//                 // Default to index.html if root
//                 if (req.path == "/") filepath = "./index.html";
//                 if (req.path == "/dashboard") filepath = "./dashboard.html"; // Clean URL mapping
                
//                 // Check if file exists
//                 struct stat buffer;
//                 if (stat(filepath.c_str(), &buffer) == 0) {
//                     // STANDARD STATIC FILE
//                     response_body = load_file(filepath);
//                     status_line = "HTTP/1.1 200 OK\r\n";
//                     response_headers = "Content-Type: " + get_mime_type(filepath) + "\r\n";
//                 } else {
//                     // 404
//                     response_body = "<h1>404 Not Found</h1>";
//                     status_line = "HTTP/1.1 404 Not Found\r\n";
//                     response_headers = "Content-Type: text/html\r\n";
//                 }
//             }
//         }
//         else {
//              status_line = "HTTP/1.1 405 Method Not Allowed\r\n";
//         }

//         // --- 3. CONSTRUCT & SEND ---
//         std::string final_response = status_line + 
//                                      set_cookie_header + 
//                                      response_headers + 
//                                      "Content-Length: " + std::to_string(response_body.length()) + "\r\n" +
//                                      "Connection: close\r\n\r\n" + 
//                                      response_body;

//         send(client_fd, final_response.c_str(), final_response.length(), 0);
//     }

//     global_stats.connection_closed();
//     close(client_fd);
// }






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
    std::cout << "Switching to port " << PORT << "\n";
    PORT = 3000;
    // return 1;
  }

  // Listen 
  if (listen(server_fd, 10) < 0){
    // listen() tells the kernel to start accepting SYN packets.
    std::cerr << "Listen failed";
    return 1;
  }

  std::cout << "Event Loop + Threadpool started.\nServer listening on port " << PORT << " ... waiting for connections. \n";
  std::cout << "Ctrl + Click on: http://localhost:" << PORT << "\n";
  // The Kernel is now handling "SYN/SYN-ACK/ACK" automatically in the background.


  while (true) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            
    if (client_fd < 0){
        throw std::runtime_error("accept failed");
    }
}


  close(server_fd);



  return 0;
}
