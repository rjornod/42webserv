#pragma once

enum class HttpParserState {
    REQUEST_LINE,
    HEADERS,
    BODY,
    COMPLETE,
    ERROR
};

constexpr const char* to_string(HttpParserState state)
{
    switch (state) {
        case HttpParserState::REQUEST_LINE:    return "REQUEST_LINE";
        case HttpParserState::HEADERS:   return "HEADERS";
        case HttpParserState::BODY: return "BODY";
        case HttpParserState::COMPLETE: return "COMPLETE";
        case HttpParserState::ERROR: return "ERROR";
    }
    return "ERROR";
}