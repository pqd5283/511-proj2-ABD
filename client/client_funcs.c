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

    FILE *config = fopen("../shared/config.txt", "r");
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
    printf("Client initialized with %d servers\n", server_count);
    printf("Client server IPs:\n");
    for (int i = 0; i < server_count; i++) {
        printf("Server %d: %s\n", i, server_ips[i]);
    }
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
    printf("Client sending read to server %d at %s\n", index, server_ips[index]);
    int check = rpc_send_read(server_ips[index], &key, value, sizeof(value));
    printf("Client received read response from server %d: key =%d, value=%s\n", index, key, value);
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
    printf("Client read thread for server %d done\n", index);
    return NULL;
}

void *read_wb_thread_fn(void *arg) {
    // unpack the writeback arguments struct
    printf("writing back to server\n");
    writeback_thread_args *args = (writeback_thread_args *)arg;
    int index = args->server_index;
    int writeback_key = args->write_key;
    char *write_back_value = args->write_value;
    // send the read writeback rpc
    int check = rpc_send_read_writeback(server_ips[index], writeback_key, write_back_value);
    if(check == 0){
        // success
        printf("Client writeback to server %d done\n", index);
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

    // define arrays to hold the returned keys and values from each server
    int  key_arr[n];
    char val_arr[n][1024];

    pthread_t threads[n];
    read_thread_args args[n];

    // fill out the args and make a thread for each server that calls the previously defined read thread function
    for (int i = 0; i < n; ++i) {
        key_arr[i] = -1;
        val_arr[i][0] = '\0';

        args[i].server_index = i;
        args[i].key_arr = key_arr;
        args[i].val_arr = val_arr;

        pthread_create(&threads[i], NULL, read_thread_fn, &args[i]);
        printf("Client created read thread for server %d\n", i);
    }
    // join them, kinda want to add some sort of a condition variable + maybe a timeout in the rpc so that when a quorum is reached we can stop waiting for the rest of the servers to respond
    for (int i = 0; i < n; ++i) {
        pthread_join(threads[i], NULL);
        printf("Client joined read thread for server %d\n", i);
    }

    // copy over the value with the highest timestamp/key that we got from the servers, making sure we reached a quorum
    int successes = 0;
    int max_key = -1;
    char max_value[1024] = "";

    for (int i = 0; i < n; ++i) {
        if (key_arr[i] < 0) {
            continue; 
        }
        successes++;
        if (key_arr[i] > max_key) {
            max_key = key_arr[i];
            strncpy(max_value, val_arr[i], sizeof(max_value) - 1);
            max_value[sizeof(max_value) - 1] = '\0';
            printf("Client found new max key %d with value %s from server %d\n", max_key, max_value, i);
        }
    }

    if (successes < q) {
        // quorum not reached
        return -1;
    }

    // write back the max key/value to all servers
    pthread_t writeback_threads[n];
    writeback_thread_args writeback_args[n];
    // setup and create the writeback threads
    for (int i = 0; i < n; ++i) {
        writeback_args[i].server_index = i;
        writeback_args[i].write_key = max_key;
        writeback_args[i].write_value = max_value;

        pthread_create(&writeback_threads[i], NULL, read_wb_thread_fn, &writeback_args[i]);
        printf("Client created writeback thread for server %d\n", i);
    }
    // join the writeback threads need to add an ack check later and timeout thing too maybe 
    for (int i = 0; i < n; ++i) {
        pthread_join(writeback_threads[i], NULL);
    }
    printf("Read value: %s with timestamp %d\n", max_value, max_key);
    return 0;
};


// the write is honestly almost identical to the read as we need to read from a quorum first to get the highest timestamp pair then writeback the new value with an incremented timestamp 
void *write_thread_fn(void *arg) {
    // unpack the read argument struct 
    printf("write received")
    read_thread_args *args = (read_thread_args *)arg;
    int index = args->server_index;

    // send the read rpc and store in the arrays
    int key = 0;
    char value[1024];

    int check = rpc_send_write(server_ips[index], &key, value, sizeof(value));
    if (check == 0) {
        // success so store the returned key and empty value because we dont need it for the write
        printf("Client received write response from server %d: key =%d\n", index, key);
        args->key_arr[index] = key;
    } else {
        // failed for some reason so mark as invalid? 
        args->key_arr[index] = -1;
    }
    return NULL;
}
//nearly identical to the read thread function but for writeback
void *write_wb_thread_fn(void *arg) {
    // unpack the writeback arguments struct
    writeback_thread_args *args = (writeback_thread_args *)arg;
    int index = args->server_index;
    int writeback_key = args->write_key;
    char *write_back_value = args->write_value;
    // send the read writeback rpc
    int check = rpc_send_writeback(server_ips[index], writeback_key, write_back_value, "dummy_client_id");
    if(check == 0){
        // success
        return NULL; 
    } else {
        // failed for some reason, maybe log it later
    }
    // could use some work maybe return something to indicate success/failure
    return NULL;
}

int client_write(char *value){
    int n = server_count;
    int q = n/2 + 1;

    pthread_t threads[n];
    read_thread_args args[n];
    // same code as the read to get the highest timestamped key/value pair from a quorum of servers
    int  key_arr[n];
    char val_arr[n][1024];
    for (int i = 0; i < n; ++i) {
        key_arr[i] = -1;
        val_arr[i][0] = '\0';

        args[i].server_index = i;
        args[i].key_arr = key_arr;
        args[i].val_arr = val_arr;

        pthread_create(&threads[i], NULL, write_thread_fn, &args[i]);
    }
    for (int i = 0; i < n; ++i) {
        pthread_join(threads[i], NULL);
    }

    int successes = 0;
    int max_key = -1;

    for (int i = 0; i < n; ++i) {
        if (key_arr[i] < 0) {
            continue; 
        }
        successes++;
        if (key_arr[i] > max_key) {
            max_key = key_arr[i];
        }
    }

    if (successes < q) {
        // quorum not reached
        return -1;
    }
    // this is where it slightly differs from the read, we need to writeback the new value with an incremented key/timestamp
    pthread_t writeback_threads[n];
    writeback_thread_args writeback_args[n];

    for (int i = 0; i < n; ++i) {
        writeback_args[i].server_index = i;
        writeback_args[i].write_key = max_key + 1; // increment the key for the write
        writeback_args[i].write_value = value;

        pthread_create(&writeback_threads[i], NULL, write_wb_thread_fn, &writeback_args[i]);
    }

    // join them, need to add quorum check and timeout later 
    for (int i = 0; i < n; ++i) {
        pthread_join(writeback_threads[i], NULL);
    }
    printf("Writeback complete with timestamp %d\n", max_key + 1);
    return 0;

}

void client_cleanup(){
    if (server_ips) {
        free(server_ips);
        server_ips = NULL;
    }
    server_count = 0;
};
