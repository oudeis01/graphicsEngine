#include "OSCServer.h"
#include <iostream>
#include <cstring>

namespace gfx {

OSCServer::OSCServer(int port) : port_(port), server_(nullptr), running_(false) {
}

OSCServer::~OSCServer() {
    stop();
}

bool OSCServer::start() {
    if (running_) {
        return true;
    }
    
    // Create server
    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", port_);
    
    server_ = lo_server_new(port_str, errorHandler);
    if (!server_) {
        std::cerr << "Failed to create OSC server on port " << port_ << std::endl;
        return false;
    }
    
    // Add generic handler for all messages
    lo_server_add_method(server_, nullptr, nullptr, genericHandler, this);
    
    running_ = true;
    server_thread_ = std::thread(&OSCServer::serverThread, this);
    
    std::cout << "OSC Server started on port " << port_ << std::endl;
    return true;
}

void OSCServer::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
    
    if (server_) {
        lo_server_free(server_);
        server_ = nullptr;
    }
    
    std::cout << "OSC Server stopped" << std::endl;
}

void OSCServer::addHandler(const std::string& path, MessageHandler handler) {
    handlers_[path] = handler;
}

void OSCServer::removeHandler(const std::string& path) {
    handlers_.erase(path);
}

std::string OSCServer::getURL() const {
    if (!server_) {
        return "";
    }
    
    char* url = lo_server_get_url(server_);
    std::string result(url);
    free(url);
    return result;
}

void OSCServer::errorHandler(int num, const char* msg, const char* path) {
    std::cerr << "OSC Server error " << num << " in path " << (path ? path : "unknown") 
              << ": " << msg << std::endl;
}

int OSCServer::genericHandler(const char* path, const char* types, lo_arg** argv, 
                             int argc, lo_message msg, void* user_data) {
    OSCServer* server = static_cast<OSCServer*>(user_data);
    
    // Find handler for this path
    auto it = server->handlers_.find(path);
    if (it != server->handlers_.end()) {
        it->second(path, msg);
        return 0; // Message handled
    }
    
    // No specific handler found
    std::cout << "Unhandled OSC message: " << path << " (" << types << ")" << std::endl;
    return 1; // Message not handled
}

void OSCServer::serverThread() {
    while (running_) {
        // Process messages with timeout
        lo_server_recv_noblock(server_, 50); // 50ms timeout
    }
}

} // namespace gfx
