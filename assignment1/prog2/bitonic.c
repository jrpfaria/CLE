#include <stdio.h>
#include <stdlib.h>

#define swap(a, b) {int t = *a; *a = *b; *b = t;}

void compare_and_swap(int* val1, int* val2, int dir) {
    if (dir == (*val1 > *val2))
        swap(val1, val2);
}

void bitonic_merge(int* val, int num, int dir) {
    if (num > 1) {
        int k = num >> 1;
        for (int i = 0; i < k; i++)
            compare_and_swap(val + i, val + i + k, dir);
        bitonic_merge(val, k, dir);
        bitonic_merge(val + k, k, dir);
    }
}

void bitonic_sort(int* val, int num, int dir) {
    if (num > 1) {
        int k = num >> 1;
        bitonic_sort(val, k, 1);
        bitonic_sort(val + k, k, 0);
        bitonic_merge(val, num, dir);
    }
}

void sort(int* val, int num, int dir) {
    bitonic_sort(val, num, dir);
}

int main(int argc, char* argv[]){

    if (argc < 2){
        printf("Usage: %s <input file>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (file == NULL){
        printf("Error: Unable to open file %s\n", argv[1]);
        return 1;
    }

    int n = getw(file);
    int* numbers = malloc(n * sizeof(int));
    int i = 0;
    while(fgetc(file) != EOF){
        fseek(file, -1, SEEK_CUR);
        numbers[i++] = getw(file);
    }

    int up = 1; // 1 for ascending order, 0 for descending order
    sort(numbers, n, up);

    int flag = 0;
    for (int i = 0; i < n - 1; i++) {
        if (numbers[i] > numbers[i + 1]) {
            flag = 1;
            printf("Error: Array not sorted\n");
            break;
        }
    }

    free(numbers);
    
    return flag ? -1 : 0;
}
