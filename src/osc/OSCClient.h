#pragma once

#include <lo/lo.h>
#include <string>
#include <memory>

namespace gfx {

class OSCClient {
public:
    OSCClient();
    ~OSCClient();
    
    // Connect to a server
    bool connect(const std::string& host, int port);
    void disconnect();
    bool isConnected() const { return address_ != nullptr; }
    
    // Send messages
    bool sendMessage(const std::string& path);
    bool sendMessage(const std::string& path, int value);
    bool sendMessage(const std::string& path, float value);
    bool sendMessage(const std::string& path, const std::string& value);
    bool sendMessage(const std::string& path, int i, float f);
    bool sendMessage(const std::string& path, int i, float f, const std::string& s);
    bool sendMessage(const std::string& path, int i, const std::string& s1, const std::string& s2);
    bool sendMessage(const std::string& path, int i1, const std::string& s, int i2, const std::string& s2);
    
    // Get connection info
    std::string getHost() const { return host_; }
    int getPort() const { return port_; }
    
private:
    static void errorHandler(int num, const char* msg, const char* path);
    
    lo_address address_;
    std::string host_;
    int port_;
};

} // namespace gfx
