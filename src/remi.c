#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define HOST ((uint32_t) INADDR_LOOPBACK)
#define PORT ((uint16_t) 3000)

struct AppendBuffer {
    char *data;
    size_t len;
};

int AppendBuffer_append(struct AppendBuffer *ab, char *data, size_t len) {
    ab->data = realloc(ab->data, ab->len + len);
    if (ab->data == NULL) {
        return -1;
    }

    memcpy(ab->data + ab->len, data, len);
    ab->len += len;
    return 0;
}

void AppendBuffer_free(struct AppendBuffer *ab) {
    free(ab->data);
}

int setup_socket(uint32_t host, uint16_t port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        fprintf(stderr, "ERROR: socket() failed to create socket.\n");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        fprintf(stderr, "ERROR: setsockopt() failed to set `SO_REUSEADDR` socket option.\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = { htonl(host) }
    };

    if (bind(server_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        fprintf(stderr, "ERROR: bind() failed to bind the address to socket.\n");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) == -1) {
        fprintf(stderr, "ERROR: listen() failed to start listening to connections.\n");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

int accept_conn(int server_fd) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;

    return accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
}

char *read_request(int client_fd) {
    struct AppendBuffer ab = { NULL, 0 };

    char buf[128] = {0};
    while (1) {
        ssize_t nbytes = recv(client_fd, buf, sizeof(buf) / sizeof(buf[0]), 0);
        if (nbytes <= 0) {
            break;
        }

        if (AppendBuffer_append(&ab, buf, (size_t) nbytes) == -1) {
            fprintf(stderr, "ERROR: AppendBuffer_append() failed to append data to buffer: %s\n", buf);
            break;
        }
    }

    return ab.data;
}

int main(void) {
    int server_fd = setup_socket(HOST, PORT);

    fprintf(stdout, "INFO: remi started to listen in :%d\n", PORT);

    while (1) {
        int client_fd = accept_conn(server_fd);
        if (client_fd == -1) {
            fprintf(stderr, "ERROR: accept_conn() failed to accept a new connection\n");
            continue;
        }

        fprintf(stdout, "INFO: new connection stablished\n");

        char *payload = read_request(client_fd);
        if (payload == NULL) {
            fprintf(stdout, "WARN: read_request() receives a empty payload\n");

            close(client_fd);
            continue;
        }

        size_t payload_len = strlen(payload);
        if (payload_len < 4) {
            fprintf(stdout, "WARN: any command must have at least 4 bytes\n");

            close(client_fd);
            free(payload);
            continue;
        }

        char *token_ptr;

        char *magic = strtok_r(payload, " ", &token_ptr);
        if (strcmp(magic, "remi") != 0) {
            fprintf(stdout, "WARN: that is not a message that Remi can understand :(\n");

            close(client_fd);
            free(payload);
            continue;
        }

        char *command = strtok_r(NULL, " ", &token_ptr);
        if (command == NULL) {
            fprintf(stdout, "WARN: Remi receives a empty message\n");
        } else if (strcmp(command, "ADD") == 0) {
            char *key = strtok_r(NULL, " ", &token_ptr);
            char *val = strtok_r(NULL, " ", &token_ptr);

            fprintf(stdout, "INFO: added: { '%s': '%s' }\n", key, val);
        }

        close(client_fd);
        free(payload);
    }

    return 0;
}