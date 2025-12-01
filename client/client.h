#ifndef ABD_CLIENT_H
#define ABD_CLIENT_H

#include <stddef.h>
#include "shared/shared_abd.h"

// i think we are gonna need an init, a cleanup and a wait for response functions, maybe some helper functions to
// process data we will see
// may make a struct for the client state later if needed

// returns 0 on success -1 on failure
int client_init();


int client_read();

int client_write(char *value);

// cleans up the client, may make it so it only cleans up when a certain input by the user is made

void client_cleanup();

#endif