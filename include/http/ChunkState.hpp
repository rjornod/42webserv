#pragma once

enum class ChunkState {
  SIZE_LINE,
  DATA,
  DATA_CRLF,
  TRAILERS,
  DONE,
  ERROR
};

constexpr const char* to_string(ChunkState state)
{
    switch (state) {
        case ChunkState::SIZE_LINE:   return "SIZE_LINE";
        case ChunkState::DATA:   return "DATA";
        case ChunkState::DATA_CRLF:   return "DATA_CRLF";
        case ChunkState::TRAILERS:    return "TRAILERS";
        case ChunkState::DONE:    return "DONE";
        case ChunkState::ERROR:    return "ERROR";
    }
    return "ERROR";
}