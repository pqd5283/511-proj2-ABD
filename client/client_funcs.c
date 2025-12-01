#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "client.h"
#include "client_rpc.h"

int  server_count = 0;
char (*server_ips)[128];

// opens up the config file and reads in the server ips and ports 
int client_init(){
 char line[256];

    FILE *config = fopen("shared/config.txt", "r");
    if (!config) {
        return -1;
    }

    while (fgets(line, sizeof(line), config)) {
        // ignore comments or very short/blank lines
        if (line[0] == '#' || strlen(line) < 3)
            continue;

        // server_count = N
        if (strncmp(line, "server_count", 12) == 0) {
            sscanf(line, "server_count = %d", &server_count);
            server_ips = malloc(sizeof(char[128]) * server_count);
            if (!server_ips) {
                fclose(config);
                return -1;
            }
            for (int i = 0; i < server_count; i++) {
                server_ips[i][0] = '\0';
            }
            
        }
        // server# = ip:port
        else if (strncmp(line, "server", 6) == 0) {
            int index = 0;
            char addr[128];
            if (sscanf(line, "server%d = %127s", &index, addr) == 2) {
                strncpy(server_ips[index], addr, sizeof(server_ips[index]) - 1);
                server_ips[index][sizeof(server_ips[index]) - 1] = '\0';
            }
        }
    }
    fclose(config);
    return 0;

};

// for read and write i think i need to make this multi threaded so that we can contact all servers at once and wait for responses
// concurrently instead of just sending a grpc request and waiting for a response before sending the next one as thats not super efficient

int client_read(){


};

int client_write(char *value){


}

void client_cleanup(){
    
};
