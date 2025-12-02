#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "client.h"

int main(void) {
    if (client_init() != 0) {
        fprintf(stderr, "client_init failed\n");
        return 1;
    }

    printf("Type commands: read, write <value>, quit\n");

    char command[128];
    while (1) {
        if (!fgets(command, sizeof(command), stdin)) {
            break;
        }

        if (strncmp(command, "read", 4) == 0) {
            if (client_read() != 0) {
                printf("Read failed\n");
            } else {
                printf("Read succeeded\n");
            }
        } else if (strncmp(command, "write ", 6) == 0) {
            char *value = command + 6;
            value[strcspn(value, "\n")] = '\0'; // remove newline
            if (client_write(value) != 0) {
                printf("Write failed\n");
            } else {
                printf("Write succeeded\n");
            }
        } else if (strncmp(command, "quit", 4) == 0) {
            break;
        } else {
            printf("Unknown command\n");
        }
    }
    client_cleanup();
    return 0;
}
