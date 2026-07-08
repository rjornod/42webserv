#include <gtest/gtest.h>
#include "HttpParser.hpp"
#include "HttpRequest.hpp"

class HttpParserTest : public ::testing::Test {

  protected:
  
    std::string validGetRequest;
    std::string requestHelloWorld;

    void SetUp() override {
        validGetRequest =
          "GET /index.html HTTP/1.1\r\n"
          "Host: localhost\r\n"
          "User-Agent: Test\r\n"
          "\r\n";
        requestHelloWorld = 
            "GET / HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "\r\n";
    }


};

TEST_F(HttpParserTest, ParsesSimplestRequest) {
    HttpParser parser;
    parser.partialParse(requestHelloWorld);
    HttpRequest request = parser.getRequest();

    EXPECT_EQ(request.getMethod(), HttpMethod::GET);
    EXPECT_EQ(request.getURI(), "/");
    EXPECT_EQ(request.getVersion(), "HTTP/1.1");
    EXPECT_EQ(request.getHeaders().size(), 1);
}

TEST_F(HttpParserTest, ParsesGetRequestLine) {
    HttpParser parser;
    parser.partialParse(validGetRequest);
    HttpRequest req = parser.getRequest();

    EXPECT_EQ(req.getMethod(), HttpMethod::GET);
    EXPECT_EQ(req.getURI(), "/index.html");
    EXPECT_EQ(req.getVersion(), "HTTP/1.1");
}

