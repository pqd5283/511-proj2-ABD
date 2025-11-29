#ifndef SHARED_ABD_H
#define SHARED_ABD_H

#include <stdint.h>

// only shared thing for now, may have to add more to this

typedef struct {
    uint32_t timestamp;
    char *value;
} abd_data_t;

#endif