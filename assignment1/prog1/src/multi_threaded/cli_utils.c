#include <stdio.h>
#include <unistd.h>

#include "cli_utils.h"
#include "constants.h"

void print_usage() {
    fprintf(stderr, "Usage: %s [-n num_threads] [-b max_bytes] file1.txt file2.txt ...\n"
                    "  OPTIONS:\n"
                    "  -h      --- Print this help message\n"
                    "  -t      --- Number of threads to use\n"
                    "  -b      --- Maximum bytes used for each chunk\n",
            program_name);
}

int validate_num_threads(int num_threads) {
    if (num_threads < 1 || num_threads > MAX_THREADS) {
        fprintf(stderr, "%s: Invalid number of threads (must be between 1 and %d)\n", program_name, MAX_THREADS);
        return 1;
    }
    return 0;
}

int validate_max_chunk_size(int max_chunk_size) {
    if (max_chunk_size != MAX_CHUNK_SIZE_1 && max_chunk_size != MAX_CHUNK_SIZE_2) {
        fprintf(stderr, "%s: Invalid chunk size (must be %d or %d)\n", program_name, MAX_CHUNK_SIZE_1, MAX_CHUNK_SIZE_2);
        return 1;
    }
    return 0;
}

int validate_input_files(int num_files, char *file_names[]) {
    if (num_files == 0) {
        fprintf(stderr, "%s: No input files specified\n", program_name);
        return 1;
    }
    for (int i = 0; i < num_files; i++) {
        if (access(file_names[i], R_OK) != 0) {
            fprintf(stderr, "%s: Unable to read input file %s\n", program_name, file_names[i]);
            return 1;
        }
    }
    return 0;
}