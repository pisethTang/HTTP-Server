#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <string>
#include <sstream>
#include <unordered_map>
#include <iostream>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

class HttpParser {
public:
    static HttpRequest parse(const std::string& raw_request) {
        HttpRequest req;
        std::istringstream stream(raw_request);
        std::string line;

        // 1. Request Line
        if (std::getline(stream, line)) {
            std::istringstream line_stream(line);
            line_stream >> req.method >> req.path >> req.version;
        }

        // 2. Headers
        while (std::getline(stream, line) && line != "\r") {
            if (line.empty()) break;
            if (line.back() == '\r') line.pop_back();
            
            auto delimiterPos = line.find(": ");
            if (delimiterPos != std::string::npos) {
                std::string key = line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 2);
                req.headers[key] = value;
            }
        }

        // 3. Body
        std::string body_content;
        while (std::getline(stream, line)) {
            body_content += line + "\n";
        }
        req.body = body_content;

        return req;
    }
};

#endif