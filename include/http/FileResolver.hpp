#pragma once
#include "RequestContext.hpp"

class FileResolver {

  private:

  public:

    std::string resolve(const RequestContext& ctx); // Instead of string a std::filesystem::path
    Result<std::string, URIError> percentDecode(std::string_view raw);
};