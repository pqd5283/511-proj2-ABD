#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "client.h"

// simple helper to get time in seconds
static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

typedef struct {
    long read_ops;
    long write_ops;
    double read_time;
    double write_time;
    long read_fail;
    long write_fail;
} client_stats_t;

typedef struct {
    int thread_id;
    int do_write; // 1 for write, 0 for read
    client_stats_t *stats;
} worker_args_t;

void *worker_thread_fn(void *arg) {
    worker_args_t *w = (worker_args_t *)arg;
    client_stats_t *st = w->stats;

    double t0 = now_sec();
    int check;

    if (w->do_write) {
        // one write
        check = client_write("test_value");
        st->write_time += (now_sec() - t0);
        st->write_ops++;
        if (check != 0) st->write_fail++;
    } else {
        // one read
        check = client_read();
        st->read_time += (now_sec() - t0);
        st->read_ops++;
        if (check != 0) st->read_fail++;
    }

    return NULL;
}


int main(void) {
    if (client_init() != 0) {
        fprintf(stderr, "client_init failed\n");
        return 1;
    }
    int num_threads = 100;  // total "clients"
    int num_writers = (num_threads *9) / 10;  // 10% writers, 90% readers

    pthread_t threads[num_threads];
    client_stats_t stats[num_threads];
    worker_args_t args[num_threads];

    for (int i = 0; i < num_threads; ++i) {
        stats[i] = (client_stats_t){0};
        args[i].thread_id = i;
        // first num_writers threads do writes, rest do reads
        args[i].do_write = (i < num_writers) ? 1 : 0;
        args[i].stats = &stats[i];
    }

    double start = now_sec();

    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&threads[i], NULL, worker_thread_fn, &args[i]);
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    double end = now_sec();
    double total_time = end - start;

    // aggregate results 
    long total_reads = 0;
    long total_writes = 0;
    long read_fail = 0;
    long write_fail = 0;
    double read_time = 0;
    double write_time = 0;

    for (int i = 0; i < num_threads; ++i) {
        total_reads += stats[i].read_ops;
        total_writes += stats[i].write_ops;
        read_fail += stats[i].read_fail;
        write_fail += stats[i].write_fail;
        read_time += stats[i].read_time;
        write_time += stats[i].write_time;
    }

    long total_ops = total_reads + total_writes;

    printf("Total time: %.6f sec\n", total_time);
    printf("Total ops: %ld\n", total_ops);
    printf("Reads: %ld (failures: %ld)\n", total_reads, read_fail);
    printf("Writes: %ld (failures: %ld)\n", total_writes, write_fail);

    printf("Average read latency:  %.6f sec\n", read_time / total_reads);
    printf("Average write latency: %.6f sec\n", write_time / total_writes);

    printf("Throughput: %.2f ops/sec\n", (double)total_ops / total_time);

    printf("end tests, user inputs now\n");


    printf("Type commands: read, write <value>, quit\n");
    char command[128];
    while (1) {
        if (!fgets(command, sizeof(command), stdin)) {
            break;
        }

        if (strncmp(command, "read", 4) == 0) {
            if (client_read() != 0) {
                ("Read failed\n");
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
