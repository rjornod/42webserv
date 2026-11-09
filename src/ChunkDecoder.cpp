#include "ChunkDecoder.hpp"

// Parses hex digits into a size_t; returns false on empty input, non-hex characters, or overflow.
bool ChunkDecoder::parseHex(std::string_view s, size_t& out) {
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

void ChunkDecoder::decode(std::string_view buffer) {

  switch (m_state) {
    case ChunkState::SIZE_LINE: return;
    case ChunkState::DATA: return;
    case ChunkState::DATA_CRLF: return;
    case ChunkState::TRAILERS: return;
    case::ChunkState::DONE: return;
    default:
      return;
  }
}