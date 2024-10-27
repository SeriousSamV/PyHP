//
// Created by Samuel Vishesh Paul on 28/10/24.
//

#include <assert.h>
#include <string.h>
#include <_stdlib.h>

#include "../src/tiny_http/tiny_http_server_lib.h"

int main() {
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const uint8_t request[] = "GET / HTTP/1.0\r\nHost: localhost:8085\r\nUser-Agent: curl/8.7.1\r\nAccept: */*\r\n\r\n";
    const http_request* http_req = parse_http_request(request, strlen((char *)request));
    assert(http_req != nullptr);

    return EXIT_SUCCESS;
}
