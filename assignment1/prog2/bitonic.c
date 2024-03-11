#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define swap(a, b) {int t = *a; memcpy(a,b,sizeof(int)); memcpy(b,&t,sizeof(int));}

int swaps = 1;
int calls = 0;
void compare_and_swap(int* val1, int* val2, int dir){
    if (dir == (*val1 > *val2)){
        printf("Swap %d: Swapping %d and %d, on call %d\n", swaps++, *val1, *val2, calls);
        printf("val1: %d, val2: %d\n", *val1, *val2);
        swap(val1, val2);
        printf("val1: %d, val2: %d\n", *val1, *val2);
    }
}

void bitonic_merge(int* val, int low, int num, int dir){
    if (num > 1)
    {
        int k = num/2;
        for (int i = 0; i < k; i++)
            compare_and_swap(val + i, val + i + k, dir);
        bitonic_merge(val, low, k, dir);
        bitonic_merge(val, low + k, k, dir);
    }
}

void bitonic_sort(int* val, int low, int num, int dir){
    calls++;
    printf("Bitonic Sort call no.:%d; Sorting %d elements\n", calls, num);
    printf("Array: ");
    for (int i = 0; i < num; i++)
        printf("%d, ", val[i]);
    printf("\n");

    if (num > 1)
    {
        int k = num/2;
        bitonic_sort(val, low, k, 1);
        bitonic_sort(val, low + k, k, 0);
        bitonic_merge(val, low, num, dir);
    }
}

void sort(int* val, int num, int dir){
    bitonic_sort(val, 0, num, dir);
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
    for (int i = 0; i < n - 1; i++){
        if (numbers[i] <= numbers[i + 1]){
            flag = 1;
            printf("Error: Array not sorted\n");
            break;
        }
    }

    if (flag)
        for (int i = 0; i < n; i++)
            printf("%d, ", numbers[i]);

    free(numbers);
    return flag? -1 : 0;
}
