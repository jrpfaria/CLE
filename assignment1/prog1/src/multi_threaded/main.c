#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>  
#include <pthread.h>
#include <time.h>

#include "shared_region.h"
#include "cli_utils.h"
#include "constants.h"

char *program_name;

int num_files;

int num_threads;

int max_chunk_size;

int *worker_status;

int monitor_init_status;

/* 
 * Read the files in sequence and count the number of words in each file.
 * Process each file using multiple threads. You need to separate the file into chunks and process each chunk in a separate thread.
 * The chunks should be separated by whitespace characters. You can assume that the file is encoded in UTF-8. They should be approximately 4KB in size.
*/

static void *worker(void *worker_id) {
    unsigned int id = *(unsigned int *)worker_id;
    
    file_chunk_t *chunk = (file_chunk_t *)malloc(sizeof(file_chunk_t));
    chunk->buffer = (char *)malloc(max_chunk_size * sizeof(char));

    while(fetch_data_to_process(id, chunk) == 0) {
        process_chunk(id, chunk);
    }

    free(chunk->buffer);
    free(chunk);

    worker_status[id] = EXIT_SUCCESS;
    pthread_exit(&worker_status[id]);
}

static double get_delta_time(void) {
    static struct timespec t0, t1;
    t0 = t1;
    if (clock_gettime (CLOCK_MONOTONIC, &t1) != 0) { 
        perror ("clock_gettime");
        exit(EXIT_FAILURE);
    }
    return (double) (t1.tv_sec - t0.tv_sec) + 1.0e-9 * (double) (t1.tv_nsec - t0.tv_nsec);
}

int main(int argc, char *argv[]) {

    get_delta_time();
    
    program_name = argv[0];

    num_threads = DEFAULT_NUM_THREADS;
    max_chunk_size = DEFAULT_MAX_CHUNK_SIZE;

    int opt;

    while ((opt = getopt(argc, argv, "t:b:h")) != -1) {
        switch (opt) {
            case 't':
                num_threads = atoi(optarg);
                if (validate_num_threads(num_threads) != 0) exit(EXIT_FAILURE);
                break;

            case 'b':
                max_chunk_size = atoi(optarg) * 1024;                   
                if (validate_max_chunk_size(num_threads) != 0) exit(EXIT_FAILURE);
                break;

            case 'h': default:
                print_usage(argv[0]);
                exit(EXIT_SUCCESS);
                break;
        }
    }

    num_files = argc - optind;

    char *file_names[num_files];
    for (int i = 0; i < num_files; i++) file_names[i] = argv[optind + i];

    if (validate_input_files(num_files, file_names) != 0) exit(EXIT_FAILURE);

    pthread_t *worker_thread_id;
    unsigned int *worker_id;
    int *status_pointer;

    if ((worker_status = (int *)malloc(num_threads * sizeof(int))) == NULL) {
        fprintf(stderr, "%s: Unable to allocate memory for worker lock status\n", program_name);
        exit(EXIT_FAILURE);
    }

    if ((worker_thread_id = (pthread_t *)malloc(num_threads * sizeof(pthread_t))) == NULL) {
        fprintf(stderr, "%s: Unable to allocate memory for worker threads\n", program_name);
        exit(EXIT_FAILURE);
    }

    if ((worker_id = (unsigned int *)malloc(num_threads * sizeof(unsigned int))) == NULL) {
        fprintf(stderr, "%s: Unable to allocate memory for worker IDs\n", program_name);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_threads; i++) {
        worker_id[i] = i;
        if (pthread_create(&worker_thread_id[i], NULL, worker, &worker_id[i]) != 0) {
            fprintf(stderr, "%s: Unable to create worker thread\n", program_name);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(worker_thread_id[i], (void **)&status_pointer) != 0) {
            fprintf(stderr, "%s: Unable to join worker thread\n", program_name);
            exit(EXIT_FAILURE);
        }
    }

    print_results();

    printf("Elapsed time = %.6f s\n", get_delta_time());
    
    free(worker_status);
    free(worker_thread_id);
    free(worker_id);

    return EXIT_SUCCESS;
}