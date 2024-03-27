#ifndef CHARACTER_UTILS_H_
#define CHARACTER_UTILS_H_

int utf8_bytes(int first_byte);

int decode_utf8(int *buffer, int bytes);

int lower(int ch);

int normalize(int ch);

int is_alpha(int ch);

int is_number(int ch);

int is_alnum(int ch);

int is_vowel(int ch);

int is_whitespace(int ch);

int is_punctuation(int ch);

int is_separator(int ch);

int is_merger(int ch);

#endif /* CHARACTER_UTILS_H_ */
