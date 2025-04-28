#ifndef RESPONSE_H
#define RESPONSE_H

void send_response(int client_fd, const char *status, const char *content_type, const void *body, size_t body_len);
void send_404(int client_fd);
void handle_static(int client_fd, const char *path);
void handle_calc(int client_fd, const char *path);

#endif
