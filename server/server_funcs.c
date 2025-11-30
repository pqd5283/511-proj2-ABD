#include "server.h"
#include "server_rpc.h"


int server_init();

int server_receive_read(int *out_key,
                    char *out_value,
                    size_t out_value_size){}

int server_read_writeback(int *out_key,
                    char *out_value,
                    size_t out_value_size){}

int server_receive_write(int *out_key,
                    char *out_value){}

int server_write_writeback(int key,
                               char *value,
                               char *client_id){}

// int server_lock();
// int server_unlock();

void server_cleanup(){}
