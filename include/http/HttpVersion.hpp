#pragma once

enum class HttpVersion {
    HTTP_1_0,
    HTTP_1_1,
    UNSUPPORTED    
};

constexpr const char* to_string(HttpVersion method)
{
    switch (method) {
        case HttpVersion::UNSUPPORTED: return "UNSUPPORTED";
        case HttpVersion::HTTP_1_0: return "HTTP 1.0";
        case HttpVersion::HTTP_1_1: return "HTTP 1.1";
    }
    return "ERROR";
}