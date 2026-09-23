#pragma once

#include "HttpResponse.hpp"
#include <fstream>
#include <array>
#include <unistd.h>
#include <cerrno> 

enum class WriterState {
  WRITING_HEADERS,
  WRITING_BODY,
  DONE
};

enum class WriteResult {
  WOULD_BLOCK,
  COMPLETE,
  ERROR
};

class ResponseWriter {

  private:

    HttpResponse m_response;
    WriterState m_state;

    std::string m_headersBuffer;
    size_t m_headersBytesSent = 0;
    
    size_t m_bodyBytesSent = 0; // for in-memory string body, therefore we don't need a buffer

    // in case we have a filesystem path as body
    std::ifstream m_file;
    std::array<char, 8096> m_chunkBuffer{};
    size_t chunkFilled = 0;
    size_t chunkSent = 0;

  public:

    explicit ResponseWriter(HttpResponse response);
    ~ResponseWriter() {};
    
    // Called by the poll() loop whenever the socket is writable
    // Sends amap without blocking, updates the state and returns
    WriteResult writeTo(int socketFd);
    WriteResult tryWriteHeaders(int socketFd);

    bool isDone() const {return m_state == WriterState::DONE;}
    bool isWritingHeaders() const {return m_state == WriterState::WRITING_BODY;}

  
};