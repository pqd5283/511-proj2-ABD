#ifndef CLIENT_RPC_H
#define CLIENT_RPC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>  

// sends a read request to the server at specific ip
int rpc_send_read(const char *ip,
                  int *out_key,
                  char *out_value,
                  size_t out_value_size);

// sends a read-writeback request to the server at specific ip
int rpc_send_read_writeback(const char *ip,
                            int key,
                            const char *value);


// sends a write request to the server at specific ip
int rpc_send_write(const char *ip,
                   int *out_key,
                   char *out_value,
                   size_t out_value_size);


// sends a writeback to the server at specific ip 
int rpc_send_writeback(const char *ip,
                       int key,
                       const char *value,
                       const char *client_id);

// sends a lock acquire request to the server at specific ip
int rpc_acquire_lock(const char *ip,
                     const char *lock_key);
               
                     

#ifdef __cplusplus
}
#endif

#endif 
