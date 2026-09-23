#pragma once
#include "RequestContext.hpp"
#include "HttpResponse.hpp"
#include "Result.hpp"

enum class MimeTypeError {
  EXTENSION_NOT_FOUND
};

class RequestProcessor {

  private:

    // Key-value pairs of the form <file_extension> : <mime_type>
    static const std::unordered_map<std::string, std::string> M_MIME_TYPES;

  public:

    HttpResponse process(RequestContext& ctx);
    bool allowedMethod(const RequestContext& ctx);
    HttpResponse staticHandler(RequestContext& ctx);
    HttpResponse buildErrorResponse(int errorCode);
    Result<std::string,MimeTypeError>  guessMimeType(std::string file);

};