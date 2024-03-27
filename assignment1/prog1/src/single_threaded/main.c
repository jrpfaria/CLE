#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define ALPHABET_SIZE 26

int utf8_bytes(uint8_t first_byte) {
    if ((first_byte & 0b10000000) == 0) {
        return 1;
    } else if ((first_byte & 0b11100000) == 0b11000000) {
        return 2;
    } else if ((first_byte & 0b11110000) == 0b11100000) {
        return 3;
    } else if ((first_byte & 0b11111000) == 0b11110000) {
        return 4;
    }
    return -1;
}

uint32_t decode_utf8(uint8_t *buffer, int bytes) {
    uint32_t ch = 0;
    for (int i = 0; i < bytes; i++) {
        ch = (ch << 8) | buffer[i];
    }
    return ch;
}

uint32_t lower(uint32_t ch) {
    return (ch >= 0x41 && ch <= 0x5A) ? ch + 0x20 : ch;
}

uint32_t normalize(uint32_t ch) {
    if (ch == 0xC3A1 || ch == 0xC3A0 || ch == 0xC3A2 || ch == 0xC3A3 ||
        ch == 0xC381 || ch == 0xC380 || ch == 0xC382 || ch == 0xC383) {
        return 0x61;
    } 
    if (ch == 0xC3A9 || ch == 0xC3A8 || ch == 0xC3AA || ch == 0xC389 ||
        ch == 0xC388 || ch == 0xC38A) {
        return 0x65;
    }
    if (ch == 0xC3AD || ch == 0xC3AC || ch == 0xC38D || ch == 0xC38C) {
        return 0x69;
    }
    if (ch == 0xC3B3 || ch == 0xC3B2 || ch == 0xC3B4 || ch == 0xC3B5 ||
        ch == 0xC393 || ch == 0xC392 || ch == 0xC394 || ch == 0xC395) {
        return 0x6F;
    }
    if (ch == 0xC3BA || ch == 0xC3B9 || ch == 0xC39A || ch == 0xC399) {
        return 0x75;
    }
    if (ch == 0xC3A7 || ch == 0xC387) {
        return 0x63;
    }
    return lower(ch);
}

int is_alpha(uint32_t ch) {
    return (ch >= 0x41 && ch <= 0x5A) || (ch >= 0x61 && ch <= 0x7A);
}

int is_number(uint32_t ch) {
    return ch >= 0x30 && ch <= 0x39;
}

int is_alnum(uint32_t ch) {
    return (ch >= 0x41 && ch <= 0x5A) || (ch >= 0x61 && ch <= 0x7A) || (ch >= 0x30 && ch <= 0x39);
}

int is_vowel(uint32_t ch) {
    return ch == 0x61 || ch == 0x65 || ch == 0x69 || ch == 0x6F || ch == 0x75;
}

int is_whitespace(uint32_t ch) {
    return ch == 0x20 || ch == 0x09 || ch == 0x0A || ch == 0x0D;
}

int is_punctuation(uint32_t ch) {
    return ch == 0x2E || ch == 0x2C || ch == 0x3A || ch == 0x3B || ch == 0x3F || ch == 0x21 || ch == 0xE28093 || ch == 0xE280A6;
}

int is_separator(uint32_t ch) {
    return ch == 0x2D || ch == 0x22 || ch == 0x28 || ch == 0x29 || ch == 0x5B || ch == 0x5D || ch == 0xE2809C || ch == 0xE2809D;
}

int is_merger(uint32_t ch) {
    return ch == 0x27 || ch == 0xE28098 || ch == 0xE28099;
}

void read_utf8_file(const char *filename, int *word_count, int *double_consonant_count) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Failed to open file %s\n", filename);
        return;
    }

    int in_word = 0;
    int in_double_consonant = 0;
    int consonant_counts[26] = {0};

    uint8_t buffer[4096];
    size_t buffer_index = 0;
    size_t buffer_size = 0;

    while ((buffer_size = fread(buffer + buffer_index, sizeof(uint8_t), sizeof(buffer) - buffer_index, file)) > 0) {
        buffer_size += buffer_index;
        buffer_index = 0;

        for (size_t i = 0; i < buffer_size; ) {
            int bytes = utf8_bytes(buffer[i]);
            if (bytes == -1) {
                fprintf(stderr, "Error: Invalid UTF-8 sequence\n");
                fclose(file);
                return;
            }

            if (i + bytes > buffer_size) {
                memcpy(buffer, buffer + i, buffer_size - i);
                buffer_index = buffer_size - i;
                break;
            }

            uint32_t ch = decode_utf8(buffer + i, bytes);
            ch = normalize(ch);

            if (is_alnum(ch)) {
                if (!in_word) {
                    in_word = 1;
                    (*word_count)++;
                }
                if (is_alpha(ch) && !is_vowel(ch)) {
                    if (++consonant_counts[ch - 0x61] == 2 && !in_double_consonant) {
                        (*double_consonant_count)++;
                        in_double_consonant = 1;
                    }
                }
            } else if (is_whitespace(ch) || is_punctuation(ch) || is_separator(ch)) {
                if (in_word) {
                    in_word = 0;
                    in_double_consonant = 0;
                    memset(consonant_counts, 0, sizeof(consonant_counts));
                }
            }
            i += bytes;
        }
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s file1.txt file2.txt ...\n", argv[0]);
        return 1;
    }

    clock_t start = clock();
    for (int i = 1; i < argc; i++) {
        int word_count = 0;
        int double_consonant_count = 0;
        read_utf8_file(argv[i], &word_count, &double_consonant_count);
        printf("File: %s\n", argv[i]);
        printf("Total number of words: %d\n", word_count);
        printf("Number of words containing the same consonant 2 or more times: %d\n", double_consonant_count);
        printf("\n");
    }
    clock_t end = clock();
    printf("Time: %f\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    return 0;
}
