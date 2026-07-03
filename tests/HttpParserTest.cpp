#include <gtest/gtest.h>
#include "HttpParser.hpp"
#include "HttpRequest.hpp"

class HttpParserTest : public ::testing::Test {

  protected:
    // HttpParser parser;
    // HttpRequest request;
    // std::string validGetRequest;

    std::string makeGoodGetRequest() {
        return
            "GET /index.html HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "User-Agent: test\r\n"
            "\r\n";
    }

    std::string makeHelloWorld() {
        return 
            "GET / HTTP/1.1\r\n"
            "Host: localhost\r\n"
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

TEST_F(HttpParserTest, ParsesSimplestRequest) {
    HttpParser parser(makeHelloWorld());
    HttpRequest request = parser.parse();

    EXPECT_EQ(request.getMethod(), HttpMethod::GET);
    EXPECT_EQ(request.getURI(), "/");
    EXPECT_EQ(request.getVersion(), "HTTP/1.1");
    EXPECT_EQ(request.getHeaders().size(), 1);
}

TEST_F(HttpParserTest, ParsesGetRequestLine) {
    HttpParser parser(makeGoodGetRequest());
    HttpRequest req = parser.parse();

    EXPECT_EQ(req.getMethod(), HttpMethod::GET);
    EXPECT_EQ(req.getURI(), "/index.html");
    EXPECT_EQ(req.getVersion(), "HTTP/1.1");
}

