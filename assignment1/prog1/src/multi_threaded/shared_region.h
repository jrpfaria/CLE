#ifndef SHARED_REGION_H_
#define SHARED_REGION_H_

extern int num_files;

extern char **filenames;

extern int num_threads;

extern int max_chunk_size;

extern int *worker_status;

extern int monitor_init_status;

typedef struct file_data_t {
    char *filename;
    FILE *pointer;
    char previous_char;
    int num_words;
    int num_words_with_double_consonant;
} file_data_t;

typedef struct file_chunk_t {
    int file_index;
    char previous_char;
    char *buffer;
} file_chunk_t;

int fetch_data_to_process(unsigned int worker_id, file_chunk_t *file_chunk);

void print_results();

#endif /* SHARED_REGION_H_ */