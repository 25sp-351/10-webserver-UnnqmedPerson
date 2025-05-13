#include "server.h"

int main(int argc, char *argv[]) {
    int port = parse_args(argc, argv);
    start_server(port);
    return 0;
}
