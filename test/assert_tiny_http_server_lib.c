//
// Created by Samuel Vishesh Paul on 28/10/24.
//

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../src/tiny_http/tiny_http_server_lib.h"

int main() {
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const uint8_t request[] = "GET / HTTP/1.0\r\nHost: localhost:8085\r\nUser-Agent: curl/8.7.1\r\nAccept: */*\r\n\r\n";
    const http_request* http_req = parse_http_request(request, strlen((char *)request));
    assert(http_req != nullptr);
    assert(http_req->method == GET);
    assert(http_req->version == HTTP_1_0);
    assert(strncmp(http_req->url, "/", 1) == 0);
    assert(http_req->headers_cnt == 3);
    printf("Header 1: '%s': '%s'\n", http_req->headers[0]->name, http_req->headers[0]->value);
    assert(strncmp(http_req->headers[0]->name , "Host", 255) == 0);
    assert(strncmp(http_req->headers[0]->value, "localhost:8085", 255) == 0);
    printf("Header 2: '%s': '%s'\n", http_req->headers[1]->name, http_req->headers[1]->value);
    assert(strncmp(http_req->headers[1]->name, "User-Agent", 255) == 0);
    assert(strncmp(http_req->headers[1]->value, "curl/8.7.1", 255) == 0);
    printf("Header 3: '%s': '%s'\n", http_req->headers[2]->name, http_req->headers[2]->value);
    assert(strncmp(http_req->headers[2]->name, "Accept", 255) == 0);
    assert(strncmp(http_req->headers[2]->value, "*/*", 255) == 0);
    assert(http_req->body == nullptr);
    assert(http_req->body_len == 0);

    return EXIT_SUCCESS;
}
