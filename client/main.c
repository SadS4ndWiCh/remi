#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define REMI_MESSAGE_LENGTH 4
#define REMI_MESSAGE_PAYLOAD 4096

static int setup_client(uint32_t host, uint16_t port) {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        fprintf(stderr, "ERROR: socket() failed to create client socket.\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = { htonl(host) }
    };

    if (connect(client_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        fprintf(stderr, "ERROR: connect() failed to connect client to the server.\n");
        exit(EXIT_FAILURE);
    }

    return client_fd;
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

int query(int client_fd, char *payload) {
    char message[REMI_MESSAGE_LENGTH + strlen(payload) + 1];
    memset(message, 0, REMI_MESSAGE_LENGTH + strlen(payload) + 1);
    uint32_t payload_len = strlen(payload);

    memcpy(message, &payload_len, REMI_MESSAGE_LENGTH);
    memcpy(message + REMI_MESSAGE_LENGTH, payload, payload_len);

    if (rwrite(client_fd, message, REMI_MESSAGE_LENGTH + payload_len) == -1) {
        fprintf(stderr, "ERROR: rwrite() failed to send message.\n");
        return -1;
    }

    char reply_message[REMI_MESSAGE_LENGTH + REMI_MESSAGE_PAYLOAD + 1];
    if (rread(client_fd, reply_message, REMI_MESSAGE_LENGTH) == -1) {
        fprintf(stderr, "ERROR: rread() failed to read reply message length.\n");
        return -1;
    }

    uint32_t reply_len = 0;
    memcpy(&reply_len, reply_message, REMI_MESSAGE_LENGTH);
    if (reply_len > REMI_MESSAGE_PAYLOAD) {
        fprintf(stderr, "ERROR: oh no! Remi sent a too long reply!\n");
        return -1;
    }

    if (rread(client_fd, &reply_message[REMI_MESSAGE_LENGTH], reply_len) == -1) {
        fprintf(stderr, "ERROR: rread() failed to read the reply payload.\n");
        return -1;
    }

    fprintf(stdout, "<~ %s\n", &reply_message[REMI_MESSAGE_LENGTH]);

    return 0;
}

int main(void) {
    int client_fd = setup_client(INADDR_LOOPBACK, 3000);

    printf("Welcome to the Remi Client!\n\n");
    printf("Here you can send messages to Remi in an easy way! Type anything and press ENTER to send.\n");
    printf("You can exit by typing the `exit` command or just `CTRL-C`.\n");

    while (1) {
        char payload[REMI_MESSAGE_PAYLOAD];
        printf("~> ");
        fgets(payload, REMI_MESSAGE_PAYLOAD, stdin);

        if ((strlen(payload) > 0) && (payload[strlen(payload) - 1] == '\n'))
            payload[strlen(payload) - 1] = '\0';

        if (strcmp(payload, "exit") == 0) {
            break;
        }

        query(client_fd, payload);
    }

    close(client_fd);

    return EXIT_SUCCESS;
}