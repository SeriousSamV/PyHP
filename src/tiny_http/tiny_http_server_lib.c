//
// Created by Samuel Vishesh Paul on 28/10/24.
//

#include "tiny_http_server_lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 *
 * @param headers http_headers from which we've to get the Content-Length
 * @param num_headers number of headers
 * @return the value of `Content-Length` header or error (negative)
 * @retval > 0 is the actual value
 * @retval -1 headers is NULL
 * @retval -2 the `Content-Length` header is not found
 */
ssize_t get_body_size_from_header(const http_header *const *const headers, const size_t num_headers) {
    if (headers == NULL) return -1;
    for (size_t i = 0; i < num_headers; i++) {
        if (headers[i] == NULL) continue;
        if (strncmp(headers[i]->name, "Content-Length", 15) == 0) {
            return strtol(headers[i]->value, nullptr, 10);
        }
    }
    return -2;
}

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
    } else if (strncmp((char *) http_packet, "POST", 4) == 0) {
        ptr += 5; // "POST" - 5
        start_uri = 5;
        request->method = POST;
    } else {
        fprintf(stderr, "right now, only HTTP GET is supported");
        fflush(stderr);
        free(request);
        return nullptr;
    }
#ifdef DEBUG
    printf("request method: %d", request->method);
#endif

    for (int iter_cnt = 0; ptr < http_packet_len && iter_cnt < MAX_URL_LENGTH; ptr++, iter_cnt++) {
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

    if (ptr >= http_packet_len || ptr + 2 >= http_packet_len) {
        return request;
    }

    ptr += 2; // '\r\n'
    for (size_t i = 0; ptr < http_packet_len; i++) {
        if ((ptr >= http_packet_len || ptr + 1 >= http_packet_len)
            || (http_packet[ptr] == '\r'
                && http_packet[ptr + 1] == '\n')) {
            break;
        }
        http_header *header = calloc(1, sizeof(http_header));
        const size_t header_name_start_ptr = ptr;
        size_t header_name_len = 0;
        for (int iter_cnt = 0; ptr < http_packet_len && iter_cnt < MAX_HTTP_HEADER_NAME_LENGTH; ptr++, iter_cnt++) {
            if (http_packet[ptr] == ' ') {
                header_name_len = ptr - header_name_start_ptr;
                break;
            }
        }
        if (header_name_len == 0) {
            fprintf(stderr, "misformed header");
            fflush(stderr);
            free(request->url);
            free(request);
            free(header);
            return nullptr;
        }
        header->name = strndup((char *) http_packet + ptr - header_name_len, header_name_len - 1);

        for (; ptr < http_packet_len; ptr++) {
            if (http_packet[ptr] != ' ') {
                break;
            }
        }
        const size_t header_value_start = ptr;
        size_t header_value_len = 0;
        for (int iter_cnt = 0; ptr < http_packet_len && iter_cnt < MAX_HTTP_HEADER_VALUE_LENGTH; ptr++, iter_cnt++) {
            if (http_packet[ptr] == '\r' && http_packet[ptr + 1] == '\n') {
                header_value_len = ptr - header_value_start;
                break;
            }
        }
        header->value = strndup((char *) http_packet + ptr - header_value_len, header_value_len);
        if (request->headers == nullptr) {
            request->headers = calloc(1, sizeof(http_header *));
        } else {
            http_header **new_headers = realloc(request->headers, sizeof(http_header *) * (i + 1));
            if (new_headers == nullptr) {
                fprintf(stderr, "cannot allocate memory for new headers");
                fflush(stderr);
                free(request->url);
                free(request);
                free(header);
                return nullptr;
            }
            request->headers = new_headers;
        }
        request->headers[i] = header;
        request->headers_cnt = i + 1;
        ptr += 2;
    }

    ptr += 2;
    if (ptr < http_packet_len) {
        const ssize_t body_len_from_header = get_body_size_from_header(request->headers, request->headers_cnt);
        if (body_len_from_header > 0) {
            request->body_len = body_len_from_header;
        } else {
            request->body_len = http_packet_len - ptr;
        }
        request->body = (uint8_t *) strndup((char *) http_packet + ptr, request->body_len);
    }

    return request;
}
