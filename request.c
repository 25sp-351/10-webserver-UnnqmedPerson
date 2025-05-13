#include "request.h"
#include <stdio.h>
#include <string.h>

int parse_http_request(const char *request, HttpRequest *out_req) {
    return sscanf(request, "%7s %1023s %15s", out_req->method, out_req->path, out_req->protocol) == 3;
}

void url_decode(char *src) {
    char *dst = src;
    for (; *src; src++) {
        if (src[0] == '%' && src[1] == '2' && src[2] == '0') {
            *dst++ = ' ';
            src += 2;
        } else {
            *dst++ = *src;
        }
    }
    *dst = '\0';
}
