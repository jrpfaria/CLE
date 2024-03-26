#ifndef FIFO_H
#define FIFO_H

/**
 *  \brief Store a value in the data transfer region.
 *
 *  Operation carried out by the distributor.
 *
 *  \param arr pointer to the start of the subarray
 *  \param num length of the subarray
 *  \param opt sorting method (1: ascending, 0: descending) 
 *  \param act action to be performed (1: sort / 0: merge)
*/

extern void putVal (int* arr, int num, int opt, int act);

/**
 * \brief Data structure to store the necessary values in the fifo
 * as received in the putVal function
 */


typedef struct FIFO_DATA FIFO_DATA;
struct FIFO_DATA{
    int* arr;
    int num;
    int opt;
    int act;
};


/**
 *  \brief Get a value from the data transfer region.
 *
 *  Operation carried out by the workers.
 *
 *  \param workId worker identification
 *
 *  \return value
 */

extern FIFO_DATA getVal (unsigned int workId);

#endif /* FIFO_H */
