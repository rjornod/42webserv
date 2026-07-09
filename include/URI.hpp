#pragma once

#include <string>

class URI {

  public:

    URI(std::string rawPath) : m_rawPath(rawPath) {}

  private:
    // std::string m_scheme;
    std::string m_rawPath; // For now I am reading the whole thing
    
    
};