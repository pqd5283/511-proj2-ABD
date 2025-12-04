#ifndef ABD_SERVER_H
#define ABD_SERVER_H

#include <stddef.h>
#include "../shared/shared_abd.h"

// like the client i think we will need an init and cleanup, then maybe a receive read function and a 
// receive write function, eventually will add the locking functions but that can wait until i get base abd down
#ifdef __cplusplus
extern "C" {
#endif

int server_init(void);

int server_receive_read(int *out_key,
                    char *out_value,
                    size_t out_value_size);

int server_read_writeback(int key,
                    const char *out_value);

int server_receive_write(int *out_key,
                    char *out_value,
                    size_t out_value_size);

int server_write_writeback(int key,
                               const char *value,
                               const char *client_id);

// int server_lock();
// int server_unlock();

int server_acquire_lock(const char *key);

int server_release_lock(const char *key);


void server_cleanup();
#ifdef __cplusplus
}
#endif

#endif
