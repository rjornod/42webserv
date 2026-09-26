#include "ResponseWriter.hpp"

ResponseWriter::ResponseWriter(HttpResponse response) : m_response(response), m_state(WriterState::WRITING_HEADERS) {
  m_headersBuffer = m_response.getStatusLine();

  std::vector<std::pair<std::string, std::string>> headers = response.getHeaders();
  for (auto it = headers.begin(); it != headers.end(); it++) {
    m_headersBuffer += it->first + ": " + it->second + "\r\n";
  }
  m_headersBuffer += "\r\n";

}


// TO DO: check with the poll loop whether I need to close the file here in some cases
// or if it's always closed automatically with the destructor of the filestream object
WriteResult ResponseWriter::writeTo(int socketFd) {

  switch(m_state) {
    case WriterState::WRITING_HEADERS: {
      auto result = writeBuffer(socketFd, m_headersBuffer.data(), m_headersBuffer.size(), m_headersBytesSent);
      // std::cout << "Writing Headers into socket" << socketFd << std::endl;
      if (result != WriteResult::COMPLETE)
        return result;
      m_state = WriterState::WRITING_BODY;
      [[fallthrough]];
    }
    case WriterState::WRITING_BODY:
      if (std::holds_alternative<std::string>(m_response.getbodySource())) {
        const std::string& bodyStr = std::get<std::string>(m_response.getbodySource());
        auto result = writeBuffer(socketFd, bodyStr.data(), bodyStr.size(), m_bodyBytesSent);
        if (result != WriteResult::COMPLETE)
          return result;
        m_state = WriterState::DONE;
      }
      else {
        if (!m_file.is_open()) {
          const std::filesystem::path& path = std::get<std::filesystem::path>(m_response.getbodySource());
          m_file.open(path, std::ios::binary); //Prevents stream from doing newline translation which corrupts other file types
          if (!m_file.is_open())
            return WriteResult::ERROR;
        }
        while (true) {
          if (m_chunkFilled == m_chunkSent) {
            //Either at the start or current chunk fully sent
            if (!readNextChunk()) {
              m_state = WriterState::DONE; // Maybe also need to close the file?
              break; // Nothing to be read, we are done
            }
          }
          auto result = writeBuffer(socketFd, m_chunkBuffer.data(), m_chunkFilled, m_chunkSent);
          if (result != WriteResult::COMPLETE)
            return result;
        }
        
      }
      return WriteResult::COMPLETE;
    case WriterState::DONE:
      return WriteResult::COMPLETE;
    default:
      return WriteResult::COMPLETE;
  }
}

WriteResult ResponseWriter::writeBuffer(int socketFd, const char* data, size_t len, size_t& bytesSent) {
  while (bytesSent < len) {

    //MSG_NOSIGNAL suppresses the SIGPIPE signal that is received if the other end has closed the signal,
    // then it just returns a -1 and set errno to EPIPE, which we treat as any other error
    ssize_t bytesWritten = send(socketFd, data + bytesSent, len - bytesSent, MSG_NOSIGNAL);

    if (bytesWritten > 0)
      bytesSent += static_cast<size_t>(bytesWritten);
    else if (bytesWritten == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) // Not sure if this is allowed (errno)
      return WriteResult::WOULD_BLOCK;
    else if (bytesWritten == -1 && errno == EINTR) {
      continue; //Attempts to write again
    }
    else
      return WriteResult::ERROR;
  }

  return WriteResult::COMPLETE;

}

bool ResponseWriter::readNextChunk() {
  m_file.read(m_chunkBuffer.data(), m_chunkBuffer.size());
  m_chunkFilled = static_cast<size_t>(m_file.gcount()); //How many bytes were placed into the buffer
  m_chunkSent = 0; //Reset the send count
  return m_chunkFilled > 0; // Did we actually read something?
}