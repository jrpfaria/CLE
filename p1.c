#include "word_counter.c"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (file == NULL)
    {
        printf("Could not open file %s\n", argv[1]);
        return 1;
    }

    int words[2] = word_counter(file);
    printf("Number of words in file: %d\nNumber of words with repeated consonants: %d\n", words[0], words[1]);

    fclose(file);
    return 0;
}
