#include "response.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void send_response(int client_fd, const char *status, const char *content_type, const void *body, size_t body_len) {
    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.1 %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n",
             status, content_type, body_len);

    write(client_fd, header, strlen(header));
    write(client_fd, body, body_len);
}

void send_404(int client_fd) {
    const char *body = "<h1>404 Not Found</h1>";
    send_response(client_fd, "404 Not Found", "text/html", body, strlen(body));
}

void handle_calc(int client_fd, const char *path) {
    char operation[16];
    int num1, num2;
    if (sscanf(path, "/calc/%15[^/]/%d/%d", operation, &num1, &num2) == 3) {
        char body[256];
        int result = 0;
        int valid = 1;

        if (strcmp(operation, "add") == 0) result = num1 + num2;
        else if (strcmp(operation, "mul") == 0) result = num1 * num2;
        else if (strcmp(operation, "div") == 0) {
            if (num2 == 0) {
                snprintf(body, sizeof(body), "<h1>Division by zero!</h1>");
                send_response(client_fd, "400 Bad Request", "text/html", body, strlen(body));
                return;
            }
            result = num1 / num2;
        } else valid = 0;

        if (valid) {
            snprintf(body, sizeof(body), "<h1>Result: %d</h1>", result);
            send_response(client_fd, "200 OK", "text/html", body, strlen(body));
        } else {
            send_404(client_fd);
        }
    } else {
        send_404(client_fd);
    }
}

void handle_static(int client_fd, const char *path) {
    char filepath[2048];
    snprintf(filepath, sizeof(filepath), ".%s", path);

    const char *ext = strrchr(filepath, '.');
    const char *content_type = "application/octet-stream";

    if (ext) {
        if (strcmp(ext, ".html") == 0) content_type = "text/html";
        else if (strcmp(ext, ".png") == 0) content_type = "image/png";
        else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) content_type = "image/jpeg";
        else if (strcmp(ext, ".css") == 0) content_type = "text/css";
        else if (strcmp(ext, ".js") == 0) content_type = "application/javascript";
    }

    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        send_404(client_fd);
        return;
    }

    fseek(fp, 0, SEEK_END);
    size_t filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *filedata = malloc(filesize);
    if (fread(filedata, 1, filesize, fp) != filesize) {
        fclose(fp);
        free(filedata);
        send_404(client_fd);
        return;
    }
    fclose(fp);

    send_response(client_fd, "200 OK", content_type, filedata, filesize);
    free(filedata);
}
