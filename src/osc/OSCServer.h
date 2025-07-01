#pragma once

#include <lo/lo.h>
#include <string>
#include <functional>
#include <map>
#include <memory>
#include <thread>
#include <atomic>

namespace gfx {

class OSCServer {
public:
    using MessageHandler = std::function<void(const std::string& path, lo_message msg)>;
    
    OSCServer(int port);
    ~OSCServer();
    
    // Start/stop the server
    bool start();
    void stop();
    bool isRunning() const { return running_; }
    
    // Register message handlers
    void addHandler(const std::string& path, MessageHandler handler);
    void removeHandler(const std::string& path);
    
    // Get server info
    int getPort() const { return port_; }
    std::string getURL() const;
    
private:
    static void errorHandler(int num, const char* msg, const char* path);
    static int genericHandler(const char* path, const char* types, lo_arg** argv, 
                             int argc, lo_message msg, void* user_data);
    
    void serverThread();
    
    int port_;
    lo_server server_;
    std::map<std::string, MessageHandler> handlers_;
    std::atomic<bool> running_;
    std::thread server_thread_;
};

} // namespace gfx
