#pragma once
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include <filesystem>

class RequestContext {

  private:
    HttpRequest    m_httpRequest;
    const LocationConfig* m_locationConfig;
    ServerConfig   m_serverConfig; // For "global" settings
    std::filesystem::path    m_filePath;
    // std::string   m_effectiveAllowedMethods;

  public:
    RequestContext() : m_locationConfig(nullptr) {};
    
    const LocationConfig* getLocationConfig() const {return m_locationConfig;}
    ServerConfig   getServerConfig() const {return m_serverConfig;}
    // HttpRequest    getHttpRequest() const {return m_httpRequest;}
    const HttpRequest& getHttpRequest() const {return m_httpRequest;}
    std::filesystem::path    getFilePath() const {return m_filePath;}
    std::string getFilePathName() const {return m_filePath.string();}
    void           setHttpRequest(const HttpRequest& request) {m_httpRequest = request;}
    void           setServerConfig(const ServerConfig& serverConfig) {m_serverConfig = serverConfig;}
    void           setLocationConfig(const LocationConfig* locationConfig) {m_locationConfig = locationConfig;}
    void           setFilePath(std::filesystem::path path) {m_filePath = path;}
};