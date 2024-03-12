#ifndef HANDLER_H
#define HANDLER_H

#include <pthread.h>
#include "../common/fifo.h"
#include "../common/prob_const.h"

pthread_t threads[N_workers];

void handler(int *arr, int size)
{
    // divide the array into N parts

    for (int i = 0; i < N_workers; i++)
    {
        // create N threads
    }

    // loop
        // Wait for the N threads to finish their work
        // Kill half of the threads
        // k = N/2 threads merge their results with the other k threads
        // k threads sort the 2/N parts of the array

    // notify the main function that the work is finished
}

#endif