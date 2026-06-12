#pragma once

#include <string>
#include <map>

class URI;

class HttpRequest {

  private:
    std::string m_method;
    URI* m_uri; //maybe not a pointer
    std::pair<int, int> m_version;//something for the version
    std::string m_body;//something else for the body
    
    

  public: 


};