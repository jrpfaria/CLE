#include "character_utils.h"

int utf8_bytes(int first_byte) {
    if ((first_byte & 0x80) == 0) return 1;
    if ((first_byte & 0xE0) == 0xC0) return 2;
    if ((first_byte & 0xF0) == 0xE0) return 3;
    if ((first_byte & 0xF8) == 0xF0) return 4;
    return 0;
}

int decode_utf8(int *buffer, int bytes) {
    int ch = 0; for (int i = 0; i < bytes; i++) ch = (ch << 8) | buffer[i]; return ch;
}

int lower(int ch) {
    return (ch >= 0x41 && ch <= 0x5A) ? ch + 0x20 : ch;
}

int normalize(int ch) {
    if (ch == 0xC3A1 || ch == 0xC3A0 || ch == 0xC3A2 || ch == 0xC3A3 || ch == 0xC381 || ch == 0xC380 || ch == 0xC382 || ch == 0xC383) return 0x61;
    if (ch == 0xC3A9 || ch == 0xC3A8 || ch == 0xC3AA || ch == 0xC389 || ch == 0xC388 || ch == 0xC38A) return 0x65;
    if (ch == 0xC3AD || ch == 0xC3AC || ch == 0xC38D || ch == 0xC38C) return 0x69;
    if (ch == 0xC3B3 || ch == 0xC3B2 || ch == 0xC3B4 || ch == 0xC3B5 || ch == 0xC393 || ch == 0xC392 || ch == 0xC394 || ch == 0xC395) return 0x6F;
    if (ch == 0xC3BA || ch == 0xC3B9 || ch == 0xC39A || ch == 0xC399) return 0x75;
    if (ch == 0xC3A7 || ch == 0xC387) return 0x63;
    return lower(ch);
}

int is_alpha(int ch) {
    return (ch >= 0x41 && ch <= 0x5A) || (ch >= 0x61 && ch <= 0x7A);
}

int is_number(int ch) {
    return ch >= 0x30 && ch <= 0x39;
}

int is_alnum(int ch) {
    return (ch >= 0x41 && ch <= 0x5A) || (ch >= 0x61 && ch <= 0x7A) || (ch >= 0x30 && ch <= 0x39);
}

int is_vowel(int ch) {
    return ch == 0x61 || ch == 0x65 || ch == 0x69 || ch == 0x6F || ch == 0x75;
}

int is_whitespace(int ch) {
    return ch == 0x20 || ch == 0x09 || ch == 0x0A || ch == 0x0D;
}

int is_punctuation(int ch) {
    return ch == 0x2E || ch == 0x2C || ch == 0x3A || ch == 0x3B || ch == 0x3F || ch == 0x21 || ch == 0xE28093 || ch == 0xE280A6;
}

int is_separator(int ch) {
    return ch == 0x2D || ch == 0x22 || ch == 0x28 || ch == 0x29 || ch == 0x5B || ch == 0x5D || ch == 0xE2809C || ch == 0xE2809D;
}

int is_merger(int ch) {
    return ch == 0x27 || ch == 0xE28098 || ch == 0xE28099;
}