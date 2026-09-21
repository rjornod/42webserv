#include "HttpResponse.hpp"

const std::map<int, std::string> HttpResponse::M_STATUSCODES = {
  {400, "Bad Request"},
  {403, "Forbidden"},
  {404, "Not found"},
  {405, "Method Not Allowed"},
  {408, "Request Timeout"},
  {413, "Content Too Large"},
  {500, "Internal Server Error"},
  {505, "HTTP Version Not Supported"}
};