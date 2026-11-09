#pragma once
#include "ChunkState.hpp"
#include <iostream>

class ChunkDecoder {

  private:
    ChunkState m_state;
    std::size_t m_chunkRemaining;
    std::string m_body;

  public:
    void decode(std::string_view buffer);
    bool parseHex(std::string_view s, size_t& out);
};