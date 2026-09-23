#include "ResponseWriter.hpp"

ResponseWriter::ResponseWriter(HttpResponse response) : m_response(response), m_state(WriterState::WRITING_HEADERS) {
  m_headersBuffer = m_response.getStatusLine();

  std::vector<std::pair<std::string, std::string>> headers = response.getHeaders();
  for (auto it = headers.begin(); it != headers.end(); it++) {
    m_headersBuffer += it->first + ": " + it->second + "\r\n";
  }
  m_headersBuffer += "\r\n";

  std::cout << "Headers from response writer: " << m_headersBuffer << std::endl;
}

WriteResult ResponseWriter::writeTo(int socketFd) {

  switch(m_state) {
    case WriterState::WRITING_HEADERS: {
      auto result = tryWriteHeaders(socketFd);
      std::cout << "Writing Headers into socket" << socketFd << std::endl;
      if (result != WriteResult::COMPLETE)
        return result;
      m_state = WriterState::WRITING_BODY;
      [[fallthrough]];
    }
    case WriterState::WRITING_BODY:
      std::cout << "Writing Body" << std::endl;
      [[fallthrough]];
    case WriterState::DONE:
      return WriteResult::COMPLETE;
    default:
      return WriteResult::COMPLETE;
  }

  // (void)m_headersBytesSent;
  (void)m_bodyBytesSent;
  (void)m_file;
  (void)m_chunkBuffer;
  (void)chunkFilled;
  (void)chunkSent;
}

WriteResult ResponseWriter::tryWriteHeaders(int socketFd) {

  size_t lenHeaders = m_headersBuffer.size();

  const char* data = m_headersBuffer.data();

  while (m_headersBytesSent < lenHeaders) {

    ssize_t bytesWritten = write(socketFd, data + m_headersBytesSent, lenHeaders - m_headersBytesSent);

    if (bytesWritten > 0)
      m_headersBytesSent += static_cast<size_t>(bytesWritten);
    else if (bytesWritten == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) // Not sure if this is allowed (errno)
      return WriteResult::WOULD_BLOCK;
    else if (bytesWritten == -1 && errno == EINTR)
      continue; //Attempts to write again
    else
      return WriteResult::ERROR;
  }

  return WriteResult::COMPLETE;

}