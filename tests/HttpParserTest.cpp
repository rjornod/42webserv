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

//TO DO -- change this test to be more general and write smaller ones to test stages
TEST_F(HttpParserTest, ParsesRequestReceivedInChunks) {
    HttpParser parser;
    
    std::string reqLine0 =
        "GET /index.html";

    std::string reqLine1 =
        " HTTP/1.1\r\n";
    
    parser.partialParse(reqLine0);

    EXPECT_EQ(parser.getParserState(), HttpParserState::REQUEST_LINE);

    parser.partialParse(reqLine1);

    EXPECT_EQ(parser.getRequest().getMethod(), HttpMethod::GET);
    EXPECT_EQ(parser.getRequest().getURI(), "/index.html");
    EXPECT_EQ(parser.getRequest().getVersion(), "HTTP/1.1");
    EXPECT_EQ(parser.getParserState(), HttpParserState::HEADERS);

    std::string headers0 = 
    "Host: loca";
    
    std::string headers1 =
        "lhost\r\n"
        "User-Agent: Test\r\n";

    std::string end = 
        "\r\n";
    
    parser.partialParse(headers0);
    parser.partialParse(headers1);

    EXPECT_EQ(parser.getRequest().getHeaders().size(), 0);
    EXPECT_EQ(parser.getParserState(), HttpParserState::HEADERS);

    parser.partialParse(end);

    EXPECT_EQ(parser.getRequest().getHeaders().size(), 2);
    EXPECT_EQ(parser.getParserState(), HttpParserState::COMPLETE);
}

