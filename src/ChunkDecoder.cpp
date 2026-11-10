#include "ChunkDecoder.hpp"

// Parses hex digits into a size_t; returns false on empty input, non-hex characters, or overflow.
bool parseHex(std::string_view s, size_t& out) {
  if (s.empty() || s.size() > 16) return false;
  size_t value = 0;
  for (char c : s) {
    int d;
    if (c >= '0' && c <= '9') d = c - '0';
    else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
    else return false;
    value = value * 16 + static_cast<size_t>(d);
  }
  out = value;
  return true;
}

ChunkDecoder::Status ChunkDecoder::feed(std::string& buffer) {

  if (m_state == ChunkState::ERROR)
    return Status::ERROR;

  while (m_state != ChunkState::DONE) {
    switch (m_state) {

      case ChunkState::SIZE_LINE: {
        size_t crlf = buffer.find("\r\n");
        if (crlf == std::string::npos) {
          if (buffer.size() > MAX_LINE_LEN) // We put a limit to the size of lines
            return fail(400);
          return ChunkDecoder::Status::NEED_MORE;
        }
      
        std::string_view line(buffer.data(), crlf);
        size_t semi = line.find(";");
        if (semi != std::string_view::npos)
          line= line.substr(0, semi); // Ignore chunk "extensions" i.e. everything after ; and before crlf
        size_t len;
        if (!parseHex(line, len))
          return fail(400);

        buffer.erase(0, crlf + 2);
        if (len == 0)
          m_state = ChunkState::TRAILERS; // Read last chunk
        else { // Read chunk size, now to read data
          m_remaining = len;
          m_state = ChunkState::DATA; 
        }
        break;
      }

      case ChunkState::DATA: {
        if (buffer.empty())
          return ChunkDecoder::Status::NEED_MORE;
        size_t take = std::min(m_remaining, buffer.size());
        if (take == 0)
          return ChunkDecoder::Status::NEED_MORE;
        if (m_body.size() + take > m_maxBodySize)
          return fail(413);
        m_body.append(buffer, 0, take);
        buffer.erase(0, take);
        m_remaining -= take;
        if (m_remaining == 0)
          m_state = ChunkState::DATA_CRLF;
        break;
      }

      case ChunkState::DATA_CRLF: {
        if (buffer.size() < 2)
          return ChunkDecoder::Status::NEED_MORE;
        if (buffer[0] != '\r' || buffer[1] != '\n')
          return fail(400);
        buffer.erase(0, 2);
        m_state = ChunkState::SIZE_LINE;
        break;
      }

      case ChunkState::TRAILERS: {
        if (buffer.empty())
          return ChunkDecoder::Status::DONE;
        size_t eol = buffer.find("\r\n");
        if (eol == std::string_view::npos) {
          if (buffer.size() > MAX_LINE_LEN)
            return fail(400);
          return ChunkDecoder::Status::NEED_MORE;
        }
        m_trailerBytes += eol + 2;
        if (m_trailerBytes > MAX_TRAILER_BYTES)
          return fail(400);
        bool lastLine = (eol == 0);
        buffer.erase(0, eol + 2);
        if (lastLine)
          m_state = ChunkState::DONE;
        break;
      }
      
      default: break;
    }
  }
  return ChunkDecoder::Status::DONE;
}

void ChunkDecoder::reset(size_t maxBodySize) {
  m_state = ChunkState::SIZE_LINE;
  m_remaining = 0;
  m_maxBodySize = maxBodySize;
  m_trailerBytes = 0;
  m_body = "";
  m_errorStatus = 0;
}