#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define RECEIVE_BUFFER_SIZE (1024 * 4)
#define SEND_BUFFER_SIZE (1024 * 4)

char *strip_control_chars(
    const char *const __counted_by(src_len) src,
    const size_t src_len,
    size_t *dst_len) {
    char *dst = malloc(src_len * sizeof(char) + 1);
    if (dst == nullptr) {
        fprintf(stderr, "Memory allocation failed in remove_cntl function");
        fflush(stderr);
        return nullptr;
    }
    *dst_len = 0;
    for (size_t i = 0; i < src_len; i++) {
        if (src[i] != '\n' && iscntrl(src[i]) != 0) {
            continue;
        }
        dst[*dst_len] = src[i];
        *dst_len += 1;
    }
    dst[*dst_len] = '\0';
    *dst_len += 1;
    return dst;
}

static bool is_accepting_connections = true;
static int active_connections = 0;

void handle_shutdown(const int signal_number) {
    printf("received signal %d\n", signal_number);
    fflush(stdout);
    is_accepting_connections = false;
    if (active_connections <= 0) {
        exit(EXIT_SUCCESS);
    }
}

int main(const int argc, const char *const *const argv) {
    printf("Tiny HTTP Server\n");
    signal(SIGINT, handle_shutdown);
    signal(SIGHUP, handle_shutdown);
    signal(SIGTERM, handle_shutdown);

    int listen_fd = 0;
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("cannot open socket");
        return EXIT_FAILURE;
    }
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(8085),
    };
    for (size_t bind_try_cnt = 0;
         bind_try_cnt < 5
         && bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0;
         bind_try_cnt++) {
        printf("Trying to bind to port %d...\n", server_addr.sin_port);
        fflush(stdout);
        sleep(2);
    }
    if (listen_fd < 0) {
        perror("cannot bind socket to :8085");
        return EXIT_FAILURE;
    }
    if (listen(listen_fd, SOMAXCONN) < 0) {
        perror("cannot listen on socket");
        return EXIT_FAILURE;
    }
    printf("listening on socket %d\n", listen_fd);
    fflush(stdout);

    uint8_t receive_buffer[RECEIVE_BUFFER_SIZE + 1];
    size_t receive_buffer_len = 0;
    char send_buffer[SEND_BUFFER_SIZE + 1];
    size_t send_buffer_len = 0;
    // ReSharper disable once CppDFALoopConditionNotUpdated
    while (is_accepting_connections) {
        printf("waiting for client connection...\n");
        fflush(stdout);
        struct sockaddr client_addr;
        socklen_t client_addr_len = 0;
        const int conn_fd = accept(listen_fd, &client_addr, &client_addr_len);
        char client_ip[INET6_ADDRSTRLEN] = {0};
        if (client_addr.sa_family == AF_INET) {
            const struct sockaddr_in *addr = (struct sockaddr_in *) &client_addr;
            inet_ntop(AF_INET, &addr->sin_addr, client_ip, INET_ADDRSTRLEN);
        } else if (client_addr.sa_family == AF_INET6) {
            const struct sockaddr_in6 *addr = (struct sockaddr_in6 *) &client_addr;
            inet_ntop(AF_INET6, &addr->sin6_addr, client_ip, INET6_ADDRSTRLEN);
        }
        printf("client connected from %s\n", client_ip);

        memset(receive_buffer, 0, RECEIVE_BUFFER_SIZE);
        errno = 0;
        while ((receive_buffer_len = read(conn_fd, receive_buffer, RECEIVE_BUFFER_SIZE)) > 0) {
            printf("received %ld bytes\n", receive_buffer_len);
            size_t clean_http_req_len = 0;
            // ReSharper disable once CppRedundantCastExpression
            char *clean_http_req = strip_control_chars((const char *) receive_buffer, receive_buffer_len,
                                                       &clean_http_req_len);
            printf("request:\n---\n%s---\n", clean_http_req);
            fflush(stdout);
            free(clean_http_req);

            if (receive_buffer[receive_buffer_len - 1] == '\n') {
                active_connections += 1;
                break;
            }
        }
        // ReSharper disable once CppDFAConstantConditions
        if (receive_buffer_len < 0) {
            // ReSharper disable once CppDFAUnreachableCode
            perror("client socket read possible failure");
            continue;
        }
        memset(send_buffer, 0, SEND_BUFFER_SIZE);
        send_buffer_len += snprintf(send_buffer, sizeof(send_buffer), "HTTP/1.0 200 OK\r\n");
        const char body[] = "Hello!\n";
        const size_t body_sz = sizeof(body);
        send_buffer_len += snprintf(send_buffer + send_buffer_len, sizeof(send_buffer) - send_buffer_len,
                                    "Content-Length: %ld\r\n", body_sz - 1);
        send_buffer_len += snprintf(send_buffer + send_buffer_len, sizeof(send_buffer) - send_buffer_len,
                                    "Content-Type: text/plain\r\n");
        send_buffer_len += snprintf(send_buffer + send_buffer_len, sizeof(send_buffer) - send_buffer_len, "\r\n%s",
                                    body);
        printf("sending %ld bytes\n", send_buffer_len);
        size_t clean_http_res_len = 0;
        char *clean_http_resp = strip_control_chars(send_buffer, send_buffer_len, &clean_http_res_len);
        printf("http response:\n%s\n", clean_http_resp);
        free(clean_http_resp);
        write(conn_fd, send_buffer, send_buffer_len);
        close(conn_fd);
        memset(receive_buffer, 0, RECEIVE_BUFFER_SIZE);
        memset(send_buffer, 0, SEND_BUFFER_SIZE);
        send_buffer_len = 0;
        active_connections -= 1;
    }

    close(listen_fd);
    return EXIT_SUCCESS;
}
