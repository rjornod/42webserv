#pragma once
#include "RequestContext.hpp"
#include "URIError.hpp"
#include "Result.hpp"
#include <filesystem>

enum class UriDecodeError {
  MALFORMED_PERCENT_ENCODING,
  INVALID_BYTE
};

enum class PathResolutionError {
  PATH_ESCAPES_ROOT,
  NOT_FOUND,
  SERVER_CONFIG_ERROR
};

enum class FileResolutionError {
  BAD_REQUEST,    // From UriDecodeError -- client sent garbage
  FORBIDDEN,      // Path escaped root
  NOT_FOUND,
//   IS_DIRECTORY_NO_INDEX_FILE, //It's a directory and no index file was found
  SERVER_ERROR
};

struct Resolution {
  enum Kind {FILE, DIRECTORY} kind;
  std::filesystem::path path;
};

class FileResolver {

//   private:

  public:

    Result<Resolution, FileResolutionError> resolve(const RequestContext& ctx); // Instead of string a std::filesystem::path
    // Result<std::string, URIError> cleanUri(std::string_view raw);
    Result<std::string, UriDecodeError> percentDecode(std::string_view raw);
    Result<std::vector<std::string_view>, PathResolutionError> normalizeSegments(std::string_view decodedUri);
    std::string joinPath(std::string_view base, std::string_view suffix);
    std::string makeFSPath(std::string root, std::vector<std::string_view> segments);
    Result<std::filesystem::path, FileResolutionError> checkWithinRoot(const std::filesystem::path& candidate, const std::filesystem::path& root);
    Result<Resolution, FileResolutionError> resolveIndex(const std::filesystem::path& directory, const std::vector<std::string>& indexCandidates, bool autoIndex);
};

constexpr const char* to_string(UriDecodeError error)
{
    switch (error) {
        case UriDecodeError::MALFORMED_PERCENT_ENCODING: return "MALFORMED_PERCENT_ENCODING";
        case UriDecodeError::INVALID_BYTE: return "INVALID_BYTE";
    }
    return "ERROR";
}

constexpr const char* to_string(PathResolutionError error)
{
    switch (error) {
        case PathResolutionError::PATH_ESCAPES_ROOT: return "PATH_ESCAPES_ROOT";
        case PathResolutionError::NOT_FOUND: return "NOT_FOUND";
        case PathResolutionError::SERVER_CONFIG_ERROR: return "SERVER_CONFIG_ERROR";
    }
    return "ERROR";
}

constexpr const char* to_string(FileResolutionError error)
{
    switch (error) {
        case FileResolutionError::BAD_REQUEST: return "BAD_REQUEST";
        case FileResolutionError::FORBIDDEN: return "FORBIDDEN";
        case FileResolutionError::NOT_FOUND: return "NOT_FOUND";
        // case FileResolutionError::IS_DIRECTORY_NO_INDEX_FILE: return "IS_DIRECTORY_NO_INDEX_FILE";
        case FileResolutionError::SERVER_ERROR: return "SERVER_ERROR";
    }
    return "ERROR";
}
