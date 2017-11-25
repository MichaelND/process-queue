#include "scheduler.hpp"

void RunClient(std::string command, std::string socket_path) {
    struct sockaddr_un server_addr { .sun_family = AF_UNIX };
    strcpy(server_addr.sun_path, socket_path.c_str());
    size_t server_len = strlen(server_addr.sun_path) + sizeof(server_addr.sun_family);

    // Create socket
    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(client_fd, (struct sockaddr *)&server_addr, server_len) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    // Open stream from server
    FILE *server_stream = fdopen(client_fd, "r+");
    if (server_stream == NULL) {
        perror("fdopen");
        exit(EXIT_FAILURE);
    }
    
    // Send command to server.
    fputs(command.c_str(), server_stream);
    fflush(server_stream);

    // Get server messages.
    char buffer[BUFSIZ];
    while (fgets(buffer, BUFSIZ, server_stream) != NULL) {    
        fputs(buffer, stdout);
    }

    fclose(server_stream);
    close(client_fd);
}
