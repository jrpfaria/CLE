#ifndef CLI_UTILS_H_
#define CLI_UTILS_H_

extern char *program_name;

void print_usage();

int validate_num_threads(int num_threads);

int validate_max_chunk_size(int max_chunk_size);

int validate_input_files(int num_files, char *file_names[]);

#endif /* CLI_UTILS_H_ */