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

typedef struct {
    int server_index; // which server index
    int *key_arr; // array of keys 
    char (*val_arr)[1024]; // array of 1024 values 
} read_thread_args;

typedef struct {
    int server_index; // which server index
    int write_key; // key to writeback
    char *write_value; // value to writeback
} writeback_thread_args;

void *read_thread_fn(void *arg) {
    // unpack the read argument struct 
    read_thread_args *args = (read_thread_args *)arg;
    int index = args->server_index;

    // send the read rpc and store in the arrays
    int key = 0;
    char value[1024];

    int check = rpc_send_read(server_ips[index], &key, value, sizeof(value));
    if (check == 0) {
        // success so store the returned values
        args->key_arr[index] = key;
        strncpy(args->val_arr[index], value, sizeof(args->val_arr[index]) - 1);
        args->val_arr[index][sizeof(args->val_arr[index]) - 1] = '\0';
    } else {
        // failed for some reason so mark as invalid? 
        args->key_arr[index] = -1;
        args->val_arr[index][0] = '\0';
    }
    return NULL;
}

void *read_wb_thread_fn(void *arg) {
    // unpack the writeback arguments struct
    writeback_thread_args *args = (writeback_thread_args *)arg;
    int idx = args->server_index;
    int writeback_key = args->write_key;
    char *write_back_value = args->write_value;
    // send the read writeback rpc
    int check = rpc_send_read_writeback(server_ips[idx], writeback_key, write_back_value);
    if(check == 0){
        // success
        return NULL; 
    } else {
        // failed for some reason, maybe log it later
    }
    // could use some work maybe return something to indicate success/failure
    return NULL;
}


int client_read(){

    int n = server_count;
    int q = n/2 + 1;


};

int client_write(char *value){


}

void client_cleanup(){
    
};
