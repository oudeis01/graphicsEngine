#include "OSCClient.h"
#include <iostream>
#include <cstdarg>

namespace gfx {

OSCClient::OSCClient() : address_(nullptr), port_(0) {
}

OSCClient::~OSCClient() {
    disconnect();
}

bool OSCClient::connect(const std::string& host, int port) {
    disconnect(); // Disconnect if already connected
    
    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", port);
    
    address_ = lo_address_new(host.c_str(), port_str);
    if (!address_) {
        std::cerr << "Failed to create OSC address for " << host << ":" << port << std::endl;
        return false;
    }
    
    host_ = host;
    port_ = port;
    
    std::cout << "OSC Client connected to " << host << ":" << port << std::endl;
    return true;
}

void OSCClient::disconnect() {
    if (address_) {
        lo_address_free(address_);
        address_ = nullptr;
        host_.clear();
        port_ = 0;
        std::cout << "OSC Client disconnected" << std::endl;
    }
}

bool OSCClient::sendMessage(const std::string& path) {
    if (!address_) {
        std::cerr << "OSC Client not connected" << std::endl;
        return false;
    }
    
    int result = lo_send(address_, path.c_str(), "");
    if (result == -1) {
        std::cerr << "Failed to send OSC message: " << path << std::endl;
        return false;
    }
    
    return true;
}

bool OSCClient::sendMessage(const std::string& path, int value) {
    if (!address_) {
        std::cerr << "OSC Client not connected" << std::endl;
        return false;
    }
    
    int result = lo_send(address_, path.c_str(), "i", value);
    if (result == -1) {
        std::cerr << "Failed to send OSC message: " << path << std::endl;
        return false;
    }
    
    return true;
}

bool OSCClient::sendMessage(const std::string& path, float value) {
    if (!address_) {
        std::cerr << "OSC Client not connected" << std::endl;
        return false;
    }
    
    int result = lo_send(address_, path.c_str(), "f", value);
    if (result == -1) {
        std::cerr << "Failed to send OSC message: " << path << std::endl;
        return false;
    }
    
    return true;
}

bool OSCClient::sendMessage(const std::string& path, const std::string& value) {
    if (!address_) {
        std::cerr << "OSC Client not connected" << std::endl;
        return false;
    }
    
    int result = lo_send(address_, path.c_str(), "s", value.c_str());
    if (result == -1) {
        std::cerr << "Failed to send OSC message: " << path << std::endl;
        return false;
    }
    
    return true;
}

bool OSCClient::sendMessage(const std::string& path, int i, float f) {
    if (!address_) {
        std::cerr << "OSC Client not connected" << std::endl;
        return false;
    }
    
    int result = lo_send(address_, path.c_str(), "if", i, f);
    if (result == -1) {
        std::cerr << "Failed to send OSC message: " << path << std::endl;
        return false;
    }
    
    return true;
}

bool OSCClient::sendMessage(const std::string& path, int i, float f, const std::string& s) {
    if (!address_) {
        std::cerr << "OSC Client not connected" << std::endl;
        return false;
    }
    
    int result = lo_send(address_, path.c_str(), "ifs", i, f, s.c_str());
    if (result == -1) {
        std::cerr << "Failed to send OSC message: " << path << std::endl;
        return false;
    }
    
    return true;
}

bool OSCClient::sendMessage(const std::string& path, int i, const std::string& s1, const std::string& s2) {
    if (!address_) {
        std::cerr << "OSC Client not connected" << std::endl;
        return false;
    }
    
    int result = lo_send(address_, path.c_str(), "iss", i, s1.c_str(), s2.c_str());
    if (result == -1) {
        std::cerr << "Failed to send OSC message: " << path << std::endl;
        return false;
    }
    
    return true;
}

bool OSCClient::sendMessage(const std::string& path, int i1, const std::string& s, int i2, const std::string& s2) {
    if (!address_) {
        std::cerr << "OSC Client not connected" << std::endl;
        return false;
    }
    
    int result = lo_send(address_, path.c_str(), "isis", i1, s.c_str(), i2, s2.c_str());
    if (result == -1) {
        std::cerr << "Failed to send OSC message: " << path << std::endl;
        return false;
    }
    
    return true;
}

void OSCClient::errorHandler(int num, const char* msg, const char* path) {
    std::cerr << "OSC Client error " << num << " in path " << (path ? path : "unknown") 
              << ": " << msg << std::endl;
}

} // namespace gfx
