#include "include/EventManager.hpp"
#include <stdexcept>
#include <unistd.h>





EpollManager::EpollManager(){
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1){
        // perror("epoll_create1");
        // exit(1);
        throw std::runtime_error("Failed to create epoll instance");
    }
}

EpollManager::~EpollManager(){
    if (epoll_fd != -1) close(epoll_fd);
}



void EpollManager::add(int fd, uint32_t event_flags){
    struct epoll_event ev;
    ev.events = event_flags;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_ctl: add");
    }
}


void EpollManager::remove(int fd){
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr) == -1){
        perror("epoll_ctl: del");
    
    }
}


int EpollManager::wait(int timeout_ms = -1){
    return epoll_wait(epoll_fd, events, 10, timeout_ms);
}


// helper to get the fd at a specific index after wait()
int EpollManager::get_fd(int index) const{
    return events[index].data.fd;
}

// helper to check what happened (e.g., EPOLLIN)
uint32_t EpollManager::get_events(int index) const{
    return events[index].events;
}