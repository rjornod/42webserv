#pragma once
#include <optional>
#include <string>
#include <string_view>
#include <sstream>

// TO DO: Re design or put this somewhere else (STRUCT?)
class MultipartPart {
  public: 
    std::string name;
    std::optional<std::string> fileName;
    std::string contentType = "text/plain";
    std::string_view data;
};

enum class MultipartError {
  INVALID_BOUNDARY,             // empty, or longer than 70 chars
  MISSING_OPENING_DELIMITER,    // body doesn't start with --boundary
  MALFORMED_DELIMITER,          // after a delimiter: neither "--" nor CRLF
  MISSING_CLOSING_DELIMITER,    // no further separator found: truncated body
  UNTERMINATED_PART_HEADERS,    // no blank line ending the part's headers
  PART_HEADERS_TOO_LARGE,       // header block over your limit
  TOO_MANY_PARTS,               // over your limit
  MALFORMED_PART_HEADER,        // a header line without ':'
  MISSING_CONTENT_DISPOSITION,
  INVALID_CONTENT_DISPOSITION,  // not "form-data", or broken quoting
  MISSING_FIELD_NAME,           // Content-Disposition without name="..."
};

constexpr const char* to_string(MultipartError error)
{
    switch (error) {
        case MultipartError::INVALID_BOUNDARY: return "INVALID_BOUNDARY";
        case MultipartError::INVALID_CONTENT_DISPOSITION: return "INVALID_CONTENT_DISPOSITION";
        case MultipartError::MALFORMED_DELIMITER: return "MALFORMED_DELIMITER";
        case MultipartError::MALFORMED_PART_HEADER: return "MALFORMED_PART_HEADER";
        case MultipartError::MISSING_CLOSING_DELIMITER: return "MISSING_CLOSING_DELIMITER";
        case MultipartError::MISSING_CONTENT_DISPOSITION: return "MISSING_CONTENT_DISPOSITION";
        case MultipartError::MISSING_FIELD_NAME: return "MISSING_FIELD_NAME";
        case MultipartError::MISSING_OPENING_DELIMITER: return "MISSING_OPENING_DELIMITER";
        case MultipartError::PART_HEADERS_TOO_LARGE: return "PART_HEADERS_TOO_LARGE";
        case MultipartError::TOO_MANY_PARTS: return "TOO_MANY_PARTS";
        case MultipartError::UNTERMINATED_PART_HEADERS: return "UNTERMINATED_PART_HEADERS";
    }
    return "ERROR";
}

// std::ostream& operator<<(std::ostream& os, const MultipartPart& part);

inline std::string to_string(const MultipartPart& part) {
    std::stringstream ss;

    ss << "MultipartPart {\n"
       << "  name: " << part.name << '\n'
       << "  fileName: "
       << (part.fileName.has_value() ? *part.fileName : "null")
       << '\n'
       << "  contentType: " << part.contentType << '\n'
       << "  data: " << part.data << '\n'
       << '}';

    return ss.str();
}