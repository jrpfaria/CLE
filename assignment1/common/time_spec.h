#include <time.h>
#include <stdio.h>
#include <stdlib.h>

static double get_delta_time(void)
{
    static struct timespec t0, t1;

    t0 = t1;
    if (clock_gettime(CLOCK_MONOTONIC, &t1) == -1) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }

    return (double) (t1.tv_sec - t0.tv_sec) + 
                    1.0e-9 * (double) (t1.tv_nsec - t0.tv_nsec);
}