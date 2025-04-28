#ifndef REQUEST_H
#define REQUEST_H

typedef struct {
    char method[8];
    char path[1024];
    char protocol[16];
} HttpRequest;

int parse_http_request(const char *request, HttpRequest *out_req);
void url_decode(char *src);

#endif
