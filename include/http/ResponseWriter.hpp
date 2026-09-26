#pragma once

#include "HttpResponse.hpp"
#include <fstream>
#include <array>
#include <unistd.h>
#include <sys/socket.h>
#include <cerrno> 

enum class WriterState {
  WRITING_HEADERS,
  WRITING_BODY,
  DONE,
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
    std::array<char, 65536> m_chunkBuffer{}; //holds the current 64 KB of data from the file
    size_t m_chunkFilled = 0; // Where the data ends in chunkBuffer
    size_t m_chunkSent = 0; //How many bytes (out of the chunkFilled) have been sent to the socket

  public:

    explicit ResponseWriter(HttpResponse response);
    ~ResponseWriter() {};
    
    // Called by the poll() loop whenever the socket is writable
    // Sends amap without blocking, updates the state and returns
    WriteResult writeTo(int socketFd);
    WriteResult writeBuffer(int socketFd, const char* data, size_t len, size_t& bytesSent);
    const WriterState& getState() const {return m_state;}

    bool isDone() const {return m_state == WriterState::DONE;}
    bool isWritingHeaders() const {return m_state == WriterState::WRITING_BODY;}
    bool readNextChunk();
  
};

constexpr const char* to_string(WriteResult result)
{
    switch (result) {
        case WriteResult::WOULD_BLOCK:    return "WOULD_BLOCK";
        case WriteResult::COMPLETE:   return "COMPLETE";
        case WriteResult::ERROR: return "ERROR";
    }
    return "ERROR";
}

constexpr const char* to_string(WriterState state)
{
    switch (state) {
        case WriterState::WRITING_HEADERS:    return "WRITING_HEADERS";
        case WriterState::WRITING_BODY:   return "WRITING_BODY";
        case WriterState::DONE: return "DONE";
    }
    return "ERROR";
}