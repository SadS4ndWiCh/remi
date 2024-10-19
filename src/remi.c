#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define REMI_HOST INADDR_LOOPBACK
#define REMI_PORT 3000
#define REMI_MAX_QUEUE 10
#define REMI_MESSAGE_LENGTH 4
#define REMI_MESSAGE_PAYLOAD 4096

static int setup_socket(uint32_t host, uint16_t port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        fprintf(stderr, "ERROR: socket() failed to create server socket.\n");
        exit(EXIT_FAILURE);
    }

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        fprintf(stderr, "ERROR: setsockopt() failed to set `SO_REUSEADDR` option.\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = { htonl(host) }
    };

    if (bind(server_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        fprintf(stderr, "ERROR: bind() failed to bind address to the socket.\n");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, REMI_MAX_QUEUE) == -1) {
        fprintf(stderr, "ERROR: listen() failed to start listening for connections.\n");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

static int rread(int fd, char *buf, size_t n) {
    while (n > 0) {
        ssize_t bytes = recv(fd, buf, n, 0);
        if (bytes == -1) {
            return -1;
        }

        if ((size_t) bytes > n) {
            return -1;
        }

        n -= bytes;
        buf += bytes;
    }

    return 0;
}

static int rwrite(int fd, const char *buf, size_t n) {
    while (n > 0) {
        ssize_t bytes = send(fd, buf, n, 0);
        if (bytes == -1) {
            return -1;
        }

        if ((size_t) bytes > n) {
            return -1;
        }

        n -= bytes;
        buf += bytes;
    }

    return 0;
}

int handle_request(int client_fd) {
    char message[REMI_MESSAGE_LENGTH + REMI_MESSAGE_PAYLOAD + 1];
    memset(message, 0, REMI_MESSAGE_LENGTH + REMI_MESSAGE_PAYLOAD + 1);
    if (rread(client_fd, message, REMI_MESSAGE_LENGTH) == -1) {
        fprintf(stderr, "ERROR: rread() failed to read request header.\n");
        return -1;
    }

    uint32_t payload_len = 0;
    memcpy(&payload_len, message, REMI_MESSAGE_LENGTH);
    if (payload_len > REMI_MESSAGE_PAYLOAD) {
        fprintf(stderr, "ERROR: client sent a too long request payload.\n");
        return -1;
    }

    if (rread(client_fd, &message[REMI_MESSAGE_LENGTH], payload_len) == -1) {
        fprintf(stderr, "ERROR: rread() failed to read the message payload.\n");
        return -1;
    }

    fprintf(stdout, "INFO: client sent: '%s'\n", &message[REMI_MESSAGE_LENGTH]);

    const char reply[] = "Good job my friend! :)";
    char replay_message[REMI_MESSAGE_LENGTH + strlen(reply)];

    uint32_t reply_len = strlen(reply);
    memcpy(replay_message, &reply_len, REMI_MESSAGE_LENGTH);
    memcpy(replay_message + REMI_MESSAGE_LENGTH, reply, reply_len);

    if (rwrite(client_fd, replay_message, REMI_MESSAGE_LENGTH + reply_len) == -1) {
        fprintf(stderr, "ERROR: rwrite() failed to send reply to client.\n");
        return -1;
    }

    return 0;
}

int main(void) {
    int server_fd = setup_socket(REMI_HOST, REMI_PORT);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            fprintf(stderr, "ERROR: accept() failed to accept client connection.\n");
            continue;
        }

        while (1) {
            if (handle_request(client_fd) == -1) {
                break;
            }
        }

        close(client_fd);
    }

    return EXIT_SUCCESS;
}