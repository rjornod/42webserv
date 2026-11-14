*This project has been created as part of the 42 curriculum by rojornod, dloustal, misiren.*

# Webserv

## Description

Webserv is a lightweight HTTP server written in C++ as part of the 42 curriculum. The project implements the core pieces of a web server from scratch: configuration parsing, TCP socket setup, non-blocking client handling, HTTP request parsing, routing, static file resolution, response generation, directory autoindexing, uploads, redirects, error pages, and CGI configuration.

The server supports multiple `server` blocks and `location` blocks in an Nginx-inspired configuration format.

### Basics of an HTTP server
An HTTP server listens for client connections, receives HTTP requests, processes them, and sends HTTP responses.

For example:

 - A browser connects to the server.
 - The server receives a request such as GET /index.html.
 - It parses the request.
 - It finds the requested file or generates an error/redirect.
 - It sends a response containing a status code, headers, and possibly a body.

In this project, the server also supports routing, configuration files, uploads, error pages, autoindexing, and CGI configuration.

## Instructions

### Requirements

- A C++ compiler with C++20 support
- GNU Make

### Build

From the repository root:

```bash
make
```

This creates the `webserv` executable. To remove build products, use `make clean`; to remove the executable as well, use `make fclean`.

### Run

The executable accepts exactly one configuration file:

```bash
./webserv configs/default-config.conf
```

The configuration file defines the listening ports, server names, document roots, indexes, allowed methods, error pages, upload locations, redirects, and other server behavior. For a minimal example, see `configs/test.conf`.

### Manual HTTP Tests
```bash
./webserv configs/default-config.conf
```
##### GET
Request home page
```bash
curl -i http://127.0.0.1:8080/
```
Request a missing file
```bash
curl -i http://127.0.0.1:8080/does-not-exist
```
##### POST (still tbd)
Create a file and upload it to the server
```bash
printf 'hello webserv\n' > /tmp/upload.txt
curl -i -X POST \
  -H "Content-Type: text/plain" \
  --data-binary @/tmp/upload.txt \
  http://127.0.0.1:8080/uploads/upload.txt
```

##### DELETE
Delete the uploaded file
```bash
curl -i -X DELETE \
  http://127.0.0.1:8080/uploads/upload.txt
```

##### Unsoported Method
```bash
curl -i -X FAKE http://127.0.0.1:8080/
```

### Automated Tests

Build all GoogleTest test binaries with:

```bash
make tests
```

Run the individual test suites from the repository root:

```bash
./config_gtests
./http_gtests
./router_gtests
./fileResolver_gtests
./chunkDecoder_gtests
```

The end-to-end test script targets `127.0.0.1:8080`. Start the server in one terminal with a configuration that listens on port 8080, then run:

```bash
python3 tests/end_to_end.py
```

## Resources

#### Webserv documentation
- [webserv: Building a Non-Blocking Web Server in C++98 (A 42 project)](https://m4nnb3ll.medium.com/webserv-building-a-non-blocking-web-server-in-c-98-a-42-project-04c7365e4ec7 ) 

- [What is a web server? - Learn web development | MDN](https://developer.mozilla.org/en-US/docs/Learn_web_development/Howto/Web_mechanics/What_is_a_web_server ) 

- [Writing an Nginx-like web server from scratch in C++](https://www.alimnaqvi.com/blog/webserv )

#### HTTP

- [HTTP RFC](https://datatracker.ietf.org/doc/html/rfc2616)
- [MDN | HTTP Guide ](https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Messages)

#### Nginx Configuration File
- [Configuring NGINX and NGINX Plus as a WebServer](https://docs.nginx.com/nginx/admin-guide/web-server/web-server/ )
- [Beginner's Guide](https://nginx.org/en/docs/beginners_guide.html)
- [The NINGX handbook](https://www.freecodecamp.org/news/the-nginx-handbook/#heading-introduction-to-nginxs-configuration-files )

#### Sockets
- [A Beginners guide to Socket Programming in C](https://dev.to/sanjayrv/a-beginners-guide-to-socket-programming-in-c-5an5 )
- [Sockets Tutorial](https://www.linuxhowtos.org/C_C++/socket.htm )

#### File descriptor monitoring (Polling)
- [IPV4 TCP server client program with Poll system call](https://www.c-pointers.com/lsp/sockets/ipv4_af_inet_tcp/poll/poll.html)

#### Non blocking I/O
- [Explain non-blocking I/O like I'm five](https://www.codecentric.de/en/knowledge-hub/blog/explain-non-blocking-i-o-like-im-five )

#### Routing
- [Configuring NGINX and NGINX Plus as a Web Server | NGINX Documentation (section Set Up Virtual Servers)](https://docs.nginx.com/nginx/admin-guide/web-server/web-server/ )

 - [How nginx processes a request](https://nginx.org/en/docs/http/request_processing.html )

- [Development guide (Section HTTP request phases)](https://nginx.org/en/docs/dev/development_guide.html#http_request)

- [NGINX Alias vs Root: When to Use Which (With Examples)](https://www.getpagespeed.com/server-setup/nginx/nginx-alias-vs-root ) 

- [Understanding NGINX URI Mapping with root, alias, and proxy_pass](https://chilankash.medium.com/understanding-nginx-uri-mapping-with-root-alias-and-proxy-pass-655d1a4d470c)

#### Responses
- [Media types (MIME types) - HTTP | MDN](https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/MIME_types)

#### File Resolution
- [Nginx Directory Indexing & Autoindex Module](https://www.cosmiclearn.com/nginx/index-and-autoindex.php)

### AI usage

AI tools were used as development aids for this project. They helped explain concepts, suggest test cases and edge cases. AI was also used to improve wording of the README.md
