#pragma once 

enum class URIError {
  MALFORMED_PERCENT_ENCODING,
  ESCAPES_ROOT,
  INVALID_BYTE,
  UNKNOWN
};

constexpr const char* to_string(URIError error)
{
    switch (error) {
        case URIError::MALFORMED_PERCENT_ENCODING: return "MALFORMED_PERCENT_ENCODING";
        case URIError::ESCAPES_ROOT: return "ESCAPES_ROOT";
        case URIError::INVALID_BYTE: return "INVALID_BYTE";
        case URIError::UNKNOWN: return "UNKNOWN";
    }
    return "ERROR";
}