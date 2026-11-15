#pragma once
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "Session.hpp"
#include <filesystem>

class RequestContext {

  private:
    HttpRequest    m_httpRequest;
    const LocationConfig* m_locationConfig;
    const ServerConfig*   m_serverConfig; // For "global" settings
    std::filesystem::path    m_filePath;
	Session& m_session;

  public:
    RequestContext(Session& session) : m_locationConfig(nullptr), m_session(session) {};
    
    const LocationConfig* getLocationConfig() const {return m_locationConfig;}
    const ServerConfig*   getServerConfig() const {return m_serverConfig;}
    HttpRequest&    getHttpRequest() {return m_httpRequest;}
    const HttpRequest& getHttpRequest() const {return m_httpRequest;}
    std::filesystem::path    getFilePath() const {return m_filePath;}
    std::string getFilePathName() const {return m_filePath.string();}
	Session& getSession() {return m_session;}
    void           setHttpRequest(const HttpRequest& request) {m_httpRequest = request;}
    void           setServerConfig(const ServerConfig* serverConfig) {m_serverConfig = serverConfig;}
    void           setLocationConfig(const LocationConfig* locationConfig) {m_locationConfig = locationConfig;}
    void           setFilePath(std::filesystem::path path) {m_filePath = path;}
    int            getClientBodySize() {if (m_locationConfig) return m_locationConfig->getMaxBodySize(); return m_serverConfig->getClientMaxBody();}
};