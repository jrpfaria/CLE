#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "shared_region.h"

static pthread_mutex_t access_critical_region = PTHREAD_MUTEX_INITIALIZER;

static pthread_once_t init = PTHREAD_ONCE_INIT;

static file_data_t *file_data;

static int current_file_index;

static void init_shared_region(void)
{
    if ((file_data = (file_data_t *)malloc(num_files * sizeof(file_data_t))) == NULL)
    {
        fprintf(stderr, "Unable to allocate memory for file data\n");
        monitor_init_status = EXIT_FAILURE;
        pthread_exit(&monitor_init_status);
    }

    for (int i = 0; i < num_files; i++)
    {
        file_data[i].filename = filenames[i];
        file_data[i].pointer = NULL;               /* Try to only have one file open at a time */
        file_data[i].previous_char = '\0';
        file_data[i].num_words = 0;
        file_data[i].num_words_with_double_consonant = 0;
    }

    current_file_index = 0;
}

int fetch_data_to_process(unsigned int worker_id, file_chunk_t *file_chunk)
{
    if ((worker_status[worker_id] = pthread_mutex_lock(&access_critical_region)) != 0)
    {
        errno = worker_status[worker_id];
        fprintf(stderr, "Worker %u: Unable to lock access to critical region\n", worker_id);
        worker_status[worker_id] = EXIT_FAILURE;
        pthread_exit(&worker_status[worker_id]);
    }

    pthread_once(&init, init_shared_region);

    /* Critical region starts here... */

    if (current_file_index < num_files)
    {
        file_data_t *file = &file_data[current_file_index];     /* Get file as a pointer so changes persist */

        if (file->pointer == NULL)
        {
            if ((file->pointer = fopen(file->filename, "rb")) == NULL)
            {
                fprintf(stderr, "Worker %u: Unable to open file %s\n", worker_id, file->filename);
                exit(EXIT_FAILURE);
            }
        }

        file_chunk->file_index = current_file_index;
        file_chunk->previous_char = file->previous_char;

        size_t bytes_read = fread(file_chunk->buffer, sizeof(char), max_chunk_size, file->pointer);
        if (bytes_read < max_chunk_size) {
            fclose(file->pointer);
            file->pointer = NULL;
            current_file_index++;
        }
    }

    if ((worker_status[worker_id] = pthread_mutex_unlock(&access_critical_region)) != 0)
    {
        errno = worker_status[worker_id];
        fprintf(stderr, "Worker %u: Unable to unlock access to critical region\n", worker_id);
        worker_status[worker_id] = EXIT_FAILURE;
        pthread_exit(&worker_status[worker_id]);
    }

    return EXIT_SUCCESS;
}

void print_results()
{
    for (int i = 0; i < num_files; i++)
    {
        printf("\nFilename: %s\n", file_data[i].filename);
        printf("Total number of words: %d\n", file_data[i].num_words);
        printf("Total number of words with at least two instances of the same consonant: %d\n", file_data[i].num_words_with_double_consonant);
    }
}