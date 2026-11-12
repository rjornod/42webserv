#pragma once
#include "RequestContext.hpp"
#include "HttpResponse.hpp"
#include "Result.hpp"
#include "FileResolver.hpp"
#include "MultipartPart.hpp"
#include <optional>
#include <unordered_set>
#include <fstream>
#include <string>

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
    HttpResponse deleteHandler(RequestContext& ctx);
    HttpResponse buildReturnResponse(int code, const std::string& target);
    HttpResponse buildAutoIndexResponse(RequestContext& ctx);
    HttpResponse buildErrorResponse(int errorCode, const RequestContext& ctx);
    HttpResponse buildDefaultErrorResponse(int errorCode);
    Result<std::string,MimeTypeError>  guessMimeType(std::string file);
    std::string generateAutoIndex(const std::string& fsPath, const std::string& uri);

    RequestContext makeErrorCtx(const RequestContext& ctx, const std::string& uri);
    const std::optional<std::string> findErrorPage(int errorCode, const RequestContext& ctx);
    bool postHeadersPresent(const HttpRequest& req);
    bool checkContentLength(const RequestContext& ctx);

    //Re factoring
    bool resolvePath(RequestContext& ctx, Resolution& out, HttpResponse& errResponse);
    HttpResponse handleGet(RequestContext& ctx);
    HttpResponse handleDelete(RequestContext& ctx);
    HttpResponse handlePost(RequestContext& ctx);
    HttpResponse handleRawUpload(RequestContext& ctx);
    HttpResponse handleMultipart(RequestContext& ctx, std::vector<MultipartPart>& parts);

    Result<std::filesystem::path, FileResolutionError> resolveForMultipart(RequestContext& ctx, std::string_view fileName);

    Result<std::vector<MultipartPart>, MultipartError> parseMultipart(std::string_view body,
                                                                  std::string_view boundary);
    Result<MultipartPart, MultipartError> parsePartHeaders(std::string_view headers);
    bool parseContentDisposition(std::string_view value, MultipartPart& part);
    bool parseContentType(const HttpRequest& req, std::unordered_map<std::string, std::string>& params, std::string& type);
    bool checkSupportedMedia(std::string type);

    int writeFileAtomically(const std::filesystem::path& target, const std::string& data);
};