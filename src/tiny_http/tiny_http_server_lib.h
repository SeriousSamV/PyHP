//
// Created by Samuel Vishesh Paul on 28/10/24.
//

#ifndef TINY_HTTP_SERVER_LIB_H
#define TINY_HTTP_SERVER_LIB_H
#include <stdint.h>
#include <stddef.h>

typedef enum http_version {
    HTTP_1_0 = 1,
} http_version;

typedef enum http_method {
    GET = 1,
    POST = 2,
} http_method;

typedef struct http_header {
    char *name;
    char *value;
} http_header;

typedef struct http_request {
    http_version version;
    http_method method;
    char *url;
    http_header **headers;
    size_t headers_cnt;
    uint8_t *body;
    size_t body_len;
    struct http_response *response;
} http_request;

typedef struct http_response {
    http_version version;
    http_header *headers;
    size_t headers_cnt;
    uint8_t *body;
    size_t body_len;
} http_response;

http_request *parse_http_request(const uint8_t *const http_packet, const size_t http_packet_len);

#endif //TINY_HTTP_SERVER_LIB_H
