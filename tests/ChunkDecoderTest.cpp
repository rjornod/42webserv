#include <gtest/gtest.h>
#include "ChunkDecoder.hpp"

class ChunkDecoderTest : public ::testing::Test {

  protected:

  // ChunkDecoder decoder;
  std::string buffer;
    

};

TEST_F(ChunkDecoderTest, DecodesCompleteRequest) {

  ChunkDecoder d(10000);
  buffer = 
    "5\r\n"
    "hello\r\n"
    "0\r\n"
    "\r\n";
  ChunkDecoder::Status status = d.feed(buffer);
  
  EXPECT_EQ(status, ChunkDecoder::Status::DONE);

}
