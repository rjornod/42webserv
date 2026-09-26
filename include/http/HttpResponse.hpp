#pragma once

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <variant>
#include <filesystem>


class HttpResponse {

  private:

    std::string m_httpVersion = "HTTP/1.1";
    int m_statusCode;
    std::string m_statusLine;
    std::vector<std::pair<std::string, std::string>> m_headers;
    std::variant<std::string, std::filesystem::path> m_bodySource;
    static const std::map<int, std::string> M_STATUSCODES;

  public:

    HttpResponse() : m_statusCode(200) {}
    HttpResponse(int statusCode) : m_statusCode(statusCode) {}

    // HttpResponse &operator=(const HttpResponse & other);

    int getStatusCode() const {return m_statusCode;}
    const std::string& getStatusLine() const {return m_statusLine;}
    const std::string& getHttpVersion() const {return m_httpVersion;}
    const std::vector<std::pair<std::string, std::string>>& getHeaders() const {return m_headers;}
    const std::variant<std::string, std::filesystem::path>& getbodySource() const {return m_bodySource;}
    const std::string& getReasonPhrase() const {return M_STATUSCODES.at(m_statusCode);}

    void setStatusCode(int status) {m_statusCode = status;}
    void setStatusLine(std::string statusLine) {m_statusLine = statusLine;}
    void setHeaders(std::vector<std::pair<std::string, std::string>> headers) {m_headers = headers;}
    void setBodySource(std::variant<std::string, std::filesystem::path> bodySource) {m_bodySource = bodySource;}

    void makeStatusLine();
    void addHeader(std::string name, std::string value);

};

std::ostream &operator<<(std::ostream &out, const HttpResponse &response);