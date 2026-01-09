#ifndef SERVER_STATS_H
#define SERVER_STATS_H

#include <mutex>
#include <string>

struct ServerStats {
    int total_requests = 0;
    int active_connections = 0;
    std::mutex stats_mutex;

    void increment_requests() {
        std::lock_guard<std::mutex> lock(stats_mutex);
        total_requests++;
    }

    void connection_opened() {
        std::lock_guard<std::mutex> lock(stats_mutex);
        active_connections++;
    }

    void connection_closed() {
        std::lock_guard<std::mutex> lock(stats_mutex);
        active_connections--;
    }

    std::string to_json() {
        std::lock_guard<std::mutex> lock(stats_mutex);
        return "{ \"requests\": " + std::to_string(total_requests) + 
               ", \"active_connections\": " + std::to_string(active_connections) + " }";
    }
};

#endif