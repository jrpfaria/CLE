#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CAPS_ASC(a, b)  if (a > b) {int t = a; a = b; b = t;}
#define CAPS_DESC(a, b) if (b > a) {int t = a; a = b; b = t;}

void bitonic_sort_asc(int* val, int num);
void bitonic_sort_desc(int* val, int num);

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

    // To do:
    // Must do a merge sort to get the numbers in the correct order
    bitonic_sort(numbers, n);
    
    for (int i = 0; i < n - 1; i++){
        if (numbers[i] > numbers[i + 1]){
            printf("Error: Array not sorted\n");
            return -1;
        }
    }

    free(numbers);
    return 0;
}


void bitonic_sort_asc(int* val, int num){

    if (num == 2){
        CAPS_ASC(val[0], val[1])
        return;
    }
    else{
        bitonic_sort_asc(val, num / 2);
        bitonic_sort_desc(val + num / 2, num / 2);
        merge(val, num);
    }
}

void bitonic_sort_desc(int* val, int num){
    if (num == 2){
        CAPS_DESC(val[0], val[1])
        return;
    }
    else{
        bitonic_sort_desc(val, num / 2);
        bitonic_sort_asc(val + num / 2, num / 2);
        merge(val, num);
    }
}