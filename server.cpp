#include "scheduler.hpp"

void RunServer(Scheduler &s, Policy policy, int microseconds) {
    int boostCount = 0;  // For priority boost in MLFQ

    // Create socket
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    // Bind socket
    struct sockaddr_un server_addr { .sun_family = AF_UNIX };
    strcpy(server_addr.sun_path,  s.GetSocketpath().c_str());
    socklen_t server_len = strlen(server_addr.sun_path) + sizeof(server_addr.sun_family);
    if (bind(server_fd, (struct sockaddr *)&server_addr, server_len) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Listen on socket
    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (true) {
        /* Poll server stream */
        struct pollfd pfd = {server_fd, POLLIN | POLLPRI, 0};
        int rc = poll(&pfd, 1, microseconds); // poll every n microsends, which is a timeslice

        /* Process event */
        if (rc < 0) {           /* Error */
            if (errno == EINTR) {
                continue;
            } else {
                fprintf(stderr, "Unable to poll: %s\n", strerror(errno));
            }
        } else if (rc == 0) {   /* Run Scheduling Algorithm */
            switch (policy) {
                case FIFO:
                    s.ServerFIFO();
                    break;
                case RDRN:
                    s.ServerRDRN();
                    break;
                case MLFQ:
                    s.ServerMLFQ(boostCount);
                    break;
            }
        } else {                /* Handle command */

            // Accept client.
            struct sockaddr_un client_addr;
            socklen_t client_len = sizeof(struct sockaddr_un);
            int       client_fd  = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
            if (client_fd < 0) {
                perror("accept");
                continue;
            }

            /* Open stream from client */
            FILE *client_stream = fdopen(client_fd, "r+");
            if (client_stream == NULL) {
                perror("fdopen");
                continue;
            }

            // Variables to determine client command.
            char buffer[BUFSIZ];
            char client_command[BUFSIZ];
            char subsequent[BUFSIZ];

            // Get client command via fgets().
            fgets(buffer, BUFSIZ, client_stream);

            // Parse command.
            sscanf(buffer, "%s %[^\n]", client_command, subsequent);
            s.SetSubsequent(subsequent);

            if (strcmp(client_command, "ADD") == 0) {
                s.HandleAddRequest(buffer, client_stream, subsequent);
            }
            else if (strcmp(client_command, "STATUS") == 0) {
                s.HandleStatusRequest(client_stream);
            }
            else if (strcmp(client_command, "RUNNING") == 0) {
                s.HandleProcessRequest(client_stream, false); // false for running
            } 
            else if (strcmp(client_command, "WAITING") == 0) {
                s.HandleProcessRequest(client_stream, true); // true for waiting
            }
            else if (strcmp(client_command, "FLUSH") == 0) {
                s.HandleFlushRequest(client_stream);
            } else {
                fputs("Command not found.\n", stderr);
            }
            close(client_fd);
            fclose(client_stream);
        }
    }
    close(server_fd);
}