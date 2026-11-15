#include "HttpRequest.hpp"

#include <sstream>
#include <algorithm>

std::ostream &operator<<(std::ostream &out, const HttpRequest &request) {

  out << "[Method] " << to_string(request.getMethod()) << std::endl;
  out << "[URI] " << request.getURI() << std::endl;
  out << "[Version] " << to_string(request.getVersion()) << std::endl;
  out << "[Headers] " << std::endl;
  // for (std::string header : request.getHeaders()) 
  //   out << header << std::endl;
  std::unordered_map<std::string, std::string> headers = request.getHeaders();
  for (auto it = headers.begin(); it != headers.end(); it++) {
    out << it->first << ": " << it->second << std::endl;
  }

  out << "[Body] " << request.getBody();

  return out;
}

// Trim from the start (in place)
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// Trim from the end (in place)
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// Trim from both ends (in place)
inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}

void HttpRequest::extractCookie() {
	m_cookies.clear();
	auto it = m_headers.find("Cookie");
	if (it == m_headers.end()) {
		return;
	}

	std::stringstream ss(it->second);
    std::string item;

    while (std::getline(ss, item, ';'))
    {
        size_t pos = item.find('=');
        if (pos != std::string::npos)
        {
            std::string key = item.substr(0, pos);
            std::string value = item.substr(pos + 1);
			trim(value);
            m_cookies[key] = value;
        }
    }
}
