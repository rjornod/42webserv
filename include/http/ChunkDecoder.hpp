#pragma once
#include "ChunkState.hpp"
#include <iostream>

class ChunkDecoder {

  public:

    enum class Status {NEED_MORE, DONE, ERROR};

    explicit ChunkDecoder(size_t maxBodySize = 0) : m_maxBodySize(maxBodySize) {}

    Status feed(std::string& buffer);
    std::string takeBody() {return std::move(m_body);}
    int getErrorStatus() const {return m_errorStatus;}
    void reset(size_t m_maxBodySize); 

  private:

    ChunkState m_state = ChunkState::SIZE_LINE;
    size_t m_remaining;
    size_t m_maxBodySize;
    size_t m_trailerBytes;
    std::string m_body;
    int m_errorStatus;

    static constexpr size_t MAX_LINE_LEN = 1024;       // size lines and trailer lines
    static constexpr size_t MAX_TRAILER_BYTES = 8192;

    Status fail(int status) {m_state = ChunkState::ERROR; m_errorStatus = status; return Status::ERROR;}
  
};