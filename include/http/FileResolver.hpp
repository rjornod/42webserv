#pragma once
#include "RequestContext.hpp"
#include "URIError.hpp"
#include "Result.hpp"

class FileResolver {

  private:

  public:

    std::string resolve(const RequestContext& ctx); // Instead of string a std::filesystem::path
    // Result<std::string, URIError> cleanUri(std::string_view raw);
    Result<std::string, URIError> percentDecode(std::string_view raw);
    Result<std::vector<std::string_view>, URIError> normalizeSegments(std::string_view decodedUri);
    std::string makeFSPath(std::string root, std::vector<std::string_view> segments);

};