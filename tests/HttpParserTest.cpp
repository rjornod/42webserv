#include <gtest/gtest.h>
#include "HttpParser.hpp"
#include "HttpRequest.hpp"

class HttpParserTest : public ::testing::Test {

  protected:

    std::string validGetRequest;
    std::string requestHelloWorld;
    std::string requestWithContentLength;
    std::string missingVersion;

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

        requestWithContentLength = 
            "POST / HTTP/1.1\r\n"
            "Host: developer.mozilla.org\r\n"
            "User-Agent: curl/8.6.0\r\n"
            "Accept: */*\r\n"
            "Content-Type: application/json\r\n"
            "content-length: 12\r\n"
            "\r\n"
            "{\"id\": \"42\"}";

        missingVersion =
            "GET /index.html\r\n"
            "Host: localhost\r\n"
            "User-Agent: Test\r\n"
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

TEST_F(HttpParserTest, ParsesChunkedRequestLine) {
    HttpParser parserComplete;
    HttpParser parserChunked;

    parserComplete.partialParse(validGetRequest);

    std::string chunk1 = "GET";
    std::string chunk2 = " /index.html ";
    std::string chunk3 = "HTTP/1.1\r\n";
    std::string rest = 
        "Host: localhost\r\n"
        "User-Agent: Test\r\n"
        "\r\n";

    parserChunked.partialParse(chunk1);
    parserChunked.partialParse(chunk2);
    parserChunked.partialParse(chunk3);
    parserChunked.partialParse(rest);
    
    EXPECT_EQ(parserComplete.getRequest().getMethod(),
        parserChunked.getRequest().getMethod());
    EXPECT_EQ(parserComplete.getRequest().getURI(),
        parserChunked.getRequest().getURI());
    EXPECT_EQ(parserComplete.getRequest().getVersion(),
        parserChunked.getRequest().getVersion());
    EXPECT_EQ(parserComplete.getRequest().getHeaders(),
        parserChunked.getRequest().getHeaders());
}

TEST_F(HttpParserTest, ParsesHeaders) {
    HttpParser parser;

    parser.partialParse(validGetRequest);

    EXPECT_EQ(parser.getRequest().getHeaders().size(), 2);
    EXPECT_EQ(parser.getRequest().getHeaders()["host"], "localhost");
    EXPECT_EQ(parser.getRequest().getHeaders()["user-agent"], "Test");
}

TEST_F(HttpParserTest, ParsesChunkedHeaders) {
    HttpParser parser;
    HttpParser parserChunked;

    parser.partialParse(validGetRequest);
    
    std::string chunk1 = "GET /index.html HTTP/1.1\r\n";
    std::string chunk2 = "Host: l";
    std::string chunk3 = "ocalhost\r\nUser-Agent: ";
    std::string chunk4 = "Test\r\n\r\n";

    parserChunked.partialParse(chunk1);
    parserChunked.partialParse(chunk2);
    parserChunked.partialParse(chunk3);
    parserChunked.partialParse(chunk4);

    std::unordered_map<std::string, std::string> headers = 
        parser.getRequest().getHeaders();
    std::unordered_map<std::string, std::string> headersChunked = 
        parserChunked.getRequest().getHeaders();

    EXPECT_EQ(headers.size(), headersChunked.size());
    EXPECT_EQ(headers["host"], headersChunked["host"]);
    EXPECT_EQ(headers["user-agent"], headersChunked["user-agent"]);

}

TEST_F(HttpParserTest, ParsesBody) {
    HttpParser parser;

    parser.partialParse(requestWithContentLength);

    EXPECT_EQ(parser.getRequest().getBody().length(),
        stoi(parser.getRequest().getHeaders()["content-length"]));
}

TEST_F(HttpParserTest, ParsesChunkedBody) {
    HttpParser parser;
    HttpParser parserChunked;

    parser.partialParse(requestWithContentLength);
    std::string chunk1 = "POST / HTTP/1.1\r\n"
            "Host: developer.mozilla.org\r\n"
            "User-Agent: curl/8.6.0\r\n"
            "Accept: */*\r\n"
            "Content-Type: application/json\r\n"
            "content-length: 12\r\n";
    std::string chunk2 = "\r\n{";
    std::string chunk3 = "\"id\": ";
    std::string chunk4 = "\"42\"}";

    parserChunked.partialParse(chunk1);
    parserChunked.partialParse(chunk2);
    parserChunked.partialParse(chunk3);
    parserChunked.partialParse(chunk4);

    EXPECT_EQ(parser.getRequest().getBody(), parserChunked.getRequest().getBody());
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

//----------------------- ERRORS -------------

// INVALID INPUT
TEST_F(HttpParserTest, MissingHTTPVersion) {
    HttpParser parser;

    parser.partialParse(missingVersion);

    EXPECT_EQ(parser.getParserState(), HttpParserState::ERROR);
}




// INCOMPLETE INPUT