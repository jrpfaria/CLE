#include "common/time_spec.h"
#include <stdint.h>
#include <string.h>

#define UTF8_BYTES(first_byte) \
    ((first_byte & 0b10000000) == 0) ? 1 : \
    ((first_byte & 0b11100000) == 0b11000000) ? 2 : \
    ((first_byte & 0b11110000) == 0b11100000) ? 3 : \
    ((first_byte & 0b11111000) == 0b11110000) ? 4 : -1

uint32_t decode_utf8(uint8_t *buffer, int bytes) {
    uint32_t ch = 0;
    for (int i = 0; i < bytes; i++) {
        ch = (ch << 8) | buffer[i];
    }
    return ch;
}

#define ALPHABET_SIZE 26

#define LOWER(ch) \
    (ch >= 0x41 && ch <= 0x5A) ? ch + 0x20 : ch

#define NORMALIZE(ch) \
    (ch == 0xC3A1 || ch == 0xC3A0 || ch == 0xC3A2 || ch == 0xC3A3 || \
     ch == 0xC381 || ch == 0xC380 || ch == 0xC382 || ch == 0xC383) ? 0x61 : \
    (ch == 0xC3A9 || ch == 0xC3A8 || ch == 0xC3AA || ch == 0xC389 || \
     ch == 0xC388 || ch == 0xC38A) ? 0x65 : \
    (ch == 0xC3AD || ch == 0xC3AC || ch == 0xC38D || ch == 0xC38C) ? 0x69 : \
    (ch == 0xC3B3 || ch == 0xC3B2 || ch == 0xC3B4 || ch == 0xC3B5 || \
     ch == 0xC393 || ch == 0xC392 || ch == 0xC394 || ch == 0xC395) ? 0x6F : \
    (ch == 0xC3BA || ch == 0xC3B9 || ch == 0xC39A || ch == 0xC399) ? 0x75 : \
    (ch == 0xC3A7 || ch == 0xC387) ? 0x63 : \
    LOWER(ch)

#define IS_ALPHA(ch) ((ch >= 0x41 && ch <= 0x5A) || (ch >= 0x61 && ch <= 0x7A))

#define IS_NUMBER(ch) (ch >= 0x30 && ch <= 0x39)

#define IS_ALNUM(ch) (IS_ALPHA(ch) || IS_NUMBER(ch))

#define IS_VOWEL(ch) (ch == 0x61 || ch == 0x65 || ch == 0x69 || ch == 0x6F || ch == 0x75)

#define IS_WHITESPACE(ch) (ch == 0x20 || ch == 0x09 || ch == 0x0A || ch == 0x0D)

#define IS_PUNCTUATION(ch) (ch == 0x2E || ch == 0x2C || ch == 0x3A || ch == 0x3B || ch == 0x3F || ch == 0x21 || ch == 0xE28093 || ch == 0xE280A6)

#define IS_SEPARATOR(ch) (ch == 0x2D || ch == 0x22 || ch == 0x28 || ch == 0x29 || ch == 0x5B || ch == 0x5D || ch == 0xE2809C || ch == 0xE2809D)

#define IS_MERGER(ch) (ch == 0x27 || ch == 0xE28098 || ch == 0xE28099)

void read_utf8_file(const char *filename, int *word_count, int *double_consonant_count) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Failed to open file %s\n", filename);
        return;
    }

    int in_word = 0;
    int in_double_consonant = 0;
    int consonant_counts[26] = {0};

    uint8_t buffer[4];
    while (!feof(file)) {
        size_t bytes_read = fread(buffer, sizeof(uint8_t), 1, file);
        if (bytes_read != 1) {
            break;
        }

        int bytes = UTF8_BYTES(buffer[0]);
        if (bytes == -1) {
            fprintf(stderr, "Error: Invalid UTF-8 sequence\n");
            break;
        }

        size_t bytes_remaining = bytes - 1;
        if (fread(buffer + 1, sizeof(uint8_t), bytes_remaining, file) != bytes_remaining) {
            break;
        }
        
        uint32_t ch = decode_utf8(buffer, bytes);

        ch = NORMALIZE(ch);

        if (IS_ALNUM(ch)) {
            if (!in_word) {
                in_word = 1;
                (*word_count)++;
            }
            if (IS_ALPHA(ch) && !IS_VOWEL(ch)) {
                if (++consonant_counts[ch - 0x61] == 2 && !in_double_consonant) {
                    (*double_consonant_count)++;
                    in_double_consonant = 1;
                }
            }
        }
        else if (IS_WHITESPACE(ch) || IS_PUNCTUATION(ch) || IS_SEPARATOR(ch)) {
            if (in_word) {
                in_word = 0;
                in_double_consonant = 0;
                memset(consonant_counts, 0, sizeof(consonant_counts));
            }
        }
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s file1.txt file2.txt ...\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        int word_count = 0;
        int double_consonant_count = 0;
        read_utf8_file(argv[i], &word_count, &double_consonant_count);
        printf("File: %s\n", argv[i]);
        printf("Total number of words: %d\n", word_count);
        printf("Number of words containing the same consonant 2 or more times: %d\n", double_consonant_count);
        printf("\n");
    }
    
    return 0;
}
