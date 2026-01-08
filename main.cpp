#include <asio/buffer.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/address.hpp>
#include <cstdlib>
#include <iostream>
#include <string.h>


// #define ASIO_STANDALONE
#include <asio.hpp> // for networking over the internet and all sorts of intersting I/Os.
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <thread>



std::vector<char> vBuffer(1 * 1024);

void GrabSomeData(asio::ip::tcp::socket& socket){
    socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()), 
    [&](std::error_code ec, std::size_t length)
    {
        if (!ec){
            std::cout << "\n\nRead " << length << " bytes\n\n" ;
            for (int i=0; i<length; i++) std::cout << vBuffer[i];

            GrabSomeData(socket);
        }
    }
);
}


int main(){
    asio::error_code ec; // to help with error-handling 

    // Create a "Context" - essentially the platform specific interface 
    asio::io_context context;

    // Give some fake tasks to asio so the context doesn't finish immediately 
    asio::io_context::work idleWork(context);



    // ===== // 
    std::thread thrContext = std::thread([&]() {context.run();});


    // Get the address of somehwere we wish to connect to 
    // std::string ipv4_addr = "142.250.70.14";
    std::string ipv4_addr = "51.38.81.49";
    int port = 80;
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address(ipv4_addr, ec), port);
    // tcp-style := (ip address, port)
    // pass in an error code ec in case our address malforms. 

    // Create a socket, the context will deliver the implementation 
    asio::ip::tcp::socket socket(context);
    // hook into the os's network driver
    // act as a doorway to the network that we connect to 

    // tell the socket to try and connect to the endpoint that we specified  
    socket.connect(endpoint, ec);

    if (!ec){
        std::cout << "Connected!" << std::endl; 
    } else {
        std::cout << "Failed to connect to address:\n" << ec.message() << std::endl; 
    }

    if (socket.is_open()){
        GrabSomeData(socket); // prime the asio context with an instruction to read data that is available 


        std::string sRequest = 
            "GET /index.html HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Connection: close\r\n\r\n"
            ;
        
            socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);
            
            // socket.wait(socket.wait_read);
            
            // size_t bytes = socket.available();
            // std::cout << "Bytes Available: " << bytes << std::endl;

            // if (bytes > 0){
            //     // if there are some bytes, thenr ead them 
            //     std::vector<char> vBuffer(bytes);
            //     socket.read_some(asio::buffer(vBuffer, vBuffer.size()), ec);
            //     for (auto c: vBuffer){
            //         std::cout << c;
            //     }
            // } else {
            //     std::cout << "Reading responsibly..." << std::endl;
            //     // std::cout << "0 bytes found ): ... " << std::endl;
            // }
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(200ms); // wait for 20s 
    }
    
    // system("pause"); for Window machine only 
    // std::cout << "Press Enter to exit ... " << std::endl;
    // std::cin.get();

    return 0;
}

// Use
// Take a thing and then prime it so that it can be used 
