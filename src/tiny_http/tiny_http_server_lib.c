//
// Created by Samuel Vishesh Paul on 28/10/24.
//

#include "tiny_http_server_lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

http_request *parse_http_request(const uint8_t *const http_packet, const size_t http_packet_len) {
    http_request *request = calloc(1, sizeof(http_request));
    if (http_packet != nullptr && http_packet_len <= 5) {
        fprintf(stderr, "cannot parse http request as it appears empty");
        fflush(stderr);
        return nullptr;
    }
    size_t ptr = 0;
    size_t start_uri = 0;
    if (strncmp((char *) http_packet, "GET", 3) == 0) {
        ptr += 4; // "GET " - 4
        start_uri = 4;
        request->method = GET;
    } else {
        fprintf(stderr, "right now, only HTTP GET is supported");
        fflush(stderr);
        free(request);
        return nullptr;
    }
#ifdef DEBUG
    printf("request method: %d", request->method);
#endif

    for (; ptr < http_packet_len; ptr++) {
        if (http_packet[ptr] == ' ') {
            request->url = strndup((char *) &http_packet[start_uri], ptr - start_uri);
            ptr++;
            break;
        }
    }
#ifdef DEBUG
    printf("request url: '%s'", request->url);
#endif
    if (ptr >= http_packet_len) {
        return request;
    }

    if (strncmp((char *) http_packet + ptr, "HTTP", 4) == 0) {
        ptr += 5; // 'HTTP/' - 5
    } else {
        fprintf(stderr, "illegal http packet");
        fflush(stderr);
        free(request->url);
        free(request);
        return nullptr;
    }
    if (strncmp((char *) http_packet + ptr, "1.0", 3) == 0) {
        request->version = HTTP_1_0;
        ptr += 3;
#ifdef DEBUG
        printf("http version: %d", request->version);
#endif
    } else {
        fprintf(stderr, "right now, only HTTP 1.0 is supported");
        fflush(stderr);
        free(request->url);
        free(request);
        return nullptr;
    }
    // TODO parse the rest

    return request;
}
