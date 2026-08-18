#pragma once

enum class HttpMethod {
    UNKNOWN,
    GET,
    POST,
    DELETE
};

constexpr const char* to_string(HttpMethod method)
{
    switch (method) {
        case HttpMethod::UNKNOWN: return "UNKNOWN";
        case HttpMethod::GET:    return "GET";
        case HttpMethod::POST:   return "POST";
        case HttpMethod::DELETE: return "DELETE";
    }
    return "ERROR";
}