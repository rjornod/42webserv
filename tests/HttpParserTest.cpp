#include <gtest/gtest.h>
#include "HttpParser.hpp"
#include "HttpRequest.hpp"

class HttpParserTest : public ::testing::Test {

  protected:
    // HttpParser parser;
    // HttpRequest request;
    // std::string validGetRequest;

    std::string makeGetRequest()
    {
        return
            "GET /index.html HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "User-Agent: test\r\n"
            "\r\n";
    }

    // void SetUp() override {
    //   validGetRequest =
    //       "GET / HTTP/1.1\r\n"
    //       "Host: localhost\r\n"
    //       "User-Agent: Test\r\n"
    //       "\r\n";
    // }


};

TEST_F(HttpParserTest, ParsesGetRequestLine)
{
    HttpParser parser(makeGetRequest());
    HttpRequest req = parser.parse();

    EXPECT_EQ(req.getMethod(), HttpMethod::GET);
    EXPECT_EQ(req.getURI(), "/index.html");
    EXPECT_EQ(req.getVersion(), "HTTP/1.1");
}

