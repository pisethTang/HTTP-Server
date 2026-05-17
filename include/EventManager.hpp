#pragma once 

#include <sys/epoll.h>

class EpollManager {
    private:
        int epoll_fd;
        struct epoll_event events[10]; // buffer 
    public:
        EpollManager();
        ~EpollManager();

        void add(int fd, uint32_t event_flags);

        void remove(int fd);

        int wait(int timeout_ms = -1);

        int get_fd(int index) const;

        uint32_t get_events(int index) const;
};