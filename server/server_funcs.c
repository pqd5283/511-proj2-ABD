#include "server.h"
#include "server_rpc.h"
#include <string.h>
#include <stdio.h>

// need a global to hold the server state (timestamp and value)
static int  timestamp = 0;
static char value[1024] = "";

// initialize the server state to 0 and empty string
int server_init(void)
{
    timestamp = 0;
    strcpy(value, "base");
    printf("server initialized \n");
    return 0;
}

// handles a read, returns current timestamp and value
int server_receive_read(int *out_key,
                        char *out_value,
                        size_t out_value_size)
{
    printf("server_receive_read called \n");
    if (out_key) {
        *out_key = timestamp;
    }
    if (out_value && out_value_size > 0) {
        strncpy(out_value, value, out_value_size - 1);
        out_value[out_value_size - 1] = '\0';
    }
    printf("server_receive_read returning key: %d, value: %s \n", timestamp, value);
    return 0;
}

// if incoming key is newer than current timestamp, update state 
int server_read_writeback(int key,
                          const char *out_value)
{
    printf("server_read_writeback called with key: %d, value: %s \n", key, out_value);
    if (!out_value) {
        return -1;
    }

    if (key > timestamp) {
        timestamp = key;
        strncpy(value, out_value, sizeof(value) - 1);
        value[sizeof(value) - 1] = '\0';
    }
    printf("server_read_writeback updated state to key: %d, value: %s \n", timestamp, value);
    return 0;
}

// write query is pretty much the same as read query because we just return current state
int server_receive_write(int *out_key,
                         char *out_value,
                         size_t out_value_size)
{
    printf("server_receive_write called \n");
    return server_receive_read(out_key, out_value, out_value_size);
}

// write writeback updates state if incoming key is newer than current timestamp (effecitvely the same as read writeback)
int server_write_writeback(int key,
                           const char *out_value,
                           const char *client_id){

    printf("server_write_writeback called from client %s \n", client_id);
    return server_read_writeback(key, out_value);
}

// just resets the state to initial values
void server_cleanup(void)
{
    timestamp = 0;
    value[0] = '\0';
}
