#include "server.h"
#include "request.h"
#include "response.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int parse_args(int argc, char *argv[]) {
    int port = 80;
    int opt;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        if (opt == 'p') {
            port = atoi(optarg);
        }
    }
    return port;
}

void *client_handler(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    HttpRequest req;
    char buffer[4096] = {0};
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        close(client_fd);
        return NULL;
    }

    if (!parse_http_request(buffer, &req)) {
        send_404(client_fd);
        close(client_fd);
        return NULL;
    }

    printf("Received: %s %s %s\n", req.method, req.path, req.protocol);

    if (strcmp(req.method, "GET") != 0) {
        send_404(client_fd);
    } else if (strncmp(req.path, "/static/", 8) == 0) {
        handle_static(client_fd, req.path);
    } else if (strncmp(req.path, "/calc/", 6) == 0) {
        handle_calc(client_fd, req.path);
    } else {
        send_404(client_fd);
    }

    close(client_fd);
    return NULL;
}

void start_server(int port) {
    signal(SIGPIPE, SIG_IGN);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int reuse = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int *client_fd = malloc(sizeof(int));
        *client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (*client_fd < 0) {
            perror("accept");
            free(client_fd);
            continue;
        }

        pthread_t tid;
        if (pthread_create(&tid, NULL, client_handler, client_fd) != 0) {
            perror("pthread_create");
            close(*client_fd);
            free(client_fd);
            continue;
        }
        pthread_detach(tid);
    }

    close(server_fd);
}
