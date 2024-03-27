#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>

#include "prob_const.h"
#include "fifo.h"

/** \brief return status on monitor initialization */
extern int statusInitMon;

/** \brief producer threads return status array */
extern int *statusDist;

/** \brief worker threads return status array */
extern int *statusWork;

/** \brief insertion pointer */
static unsigned int ii;

/** \brief retrieval pointer */
static unsigned int ri;

/** \brief flag signaling the data transfer region is full */
static bool full;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

/** \brief flag which warrants that the data transfer region is initialized exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;

/** \brief producers synchronization point when the data transfer region is full */
static pthread_cond_t fifoFull;

/** \brief workers synchronization point when the data transfer region is empty */
static pthread_cond_t fifoEmpty;

/** \brief number of storage positions in the data transfer region */
extern int nStorePos;

/** \brief storage region */
static FIFO_DATA* mem;


/**
 *  \brief Initialization of the data transfer region.
 *
 *  Internal monitor operation.
 */

static void initialization (void)
{
  if (((mem = malloc (nStorePos * sizeof(FIFO_DATA))) == NULL))
	 { fprintf (stderr, "error on allocating space to the data transfer region\n");
	   statusInitMon = EXIT_FAILURE;
	   pthread_exit (&statusInitMon);
     }

	                                                                               /* initialize FIFO in empty state */
  ii = ri = 0;                                        /* FIFO insertion and retrieval pointers set to the same value */
  full = false;                                                                                  /* FIFO is not full */

  pthread_cond_init (&fifoFull, NULL);                                 /* initialize producers synchronization point */
  pthread_cond_init (&fifoEmpty, NULL);                                /* initialize workers synchronization point */
}

/**
 *  \brief Store a value in the data transfer region.
 *
 *  Operation carried out by the worker.
 *
 *  \param arr pointer to the start of the subarray
 *  \param num length of the subarray
 *  \param opt sorting method (1: ascending, 0: descending)
 *  \param act action to be performed (1: sort / 0: merge)
 */

void putVal (int* arr, int num, int opt, int act)
{
  if ((statusDist[0] = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
     { errno = statusDist[0];                                                            /* save error in errno */
       perror ("error on entering monitor(CF)");
       statusDist[0] = EXIT_FAILURE;
       pthread_exit (&statusDist[0]);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  while (full)                                                           /* wait if the data transfer region is full */
  { if ((statusDist[0] = pthread_cond_wait (&fifoFull, &accessCR)) != 0)
       { errno = statusDist[0];                                                          /* save error in errno */
         perror ("error on waiting in fifoFull");
         statusDist[0] = EXIT_FAILURE;
         pthread_exit (&statusDist[0]);
       }
  }

  FIFO_DATA val;
  val.arr = arr;
  val.num = num;
  val.opt = opt;
  val.act = act;
  
  mem[ii] = val;                                                                          /* store value in the FIFO */
  ii = (ii + 1) % nStorePos;
  full = (ii == ri);

  if ((statusDist[0] = pthread_cond_signal (&fifoEmpty)) != 0)      /* let a worker know that a value has been
                                                                                                               stored */
     { errno = statusDist[0];                                                             /* save error in errno */
       perror ("error on signaling in fifoEmpty");
       statusDist[0] = EXIT_FAILURE;
       pthread_exit (&statusDist[0]);
     }

  if ((statusDist[0] = pthread_mutex_unlock (&accessCR)) != 0)                                  /* exit monitor */
     { errno = statusDist[0];                                                            /* save error in errno */
       perror ("error on exiting monitor(CF)");
       statusDist[0] = EXIT_FAILURE;
       pthread_exit (&statusDist[0]);
     }
}

/**
 *  \brief Get a value from the data transfer region.
 *
 *  Operation carried out by the workers.
 *
 *  \param workId worker identification
 *
 *  \return value
 */

FIFO_DATA getVal (unsigned int workId)
{
  FIFO_DATA val;                                                                               /* retrieved value */

  if ((statusWork[workId] = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
     { errno = statusWork[workId];                                                            /* save error in errno */
       perror ("error on entering monitor(CF)");
       statusWork[workId] = EXIT_FAILURE;
       pthread_exit (&statusWork[workId]);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  while ((ii == ri) && !full)                                           /* wait if the data transfer region is empty */
  { if ((statusWork[workId] = pthread_cond_wait (&fifoEmpty, &accessCR)) != 0)
       { errno = statusWork[workId];                                                          /* save error in errno */
         perror ("error on waiting in fifoEmpty");
         statusWork[workId] = EXIT_FAILURE;
         pthread_exit (&statusWork[workId]);
       }
  }

  val = mem[ri];                                                                   /* retrieve a  value from the FIFO */
  ri = (ri + 1) % nStorePos;
  full = false;

  if ((statusWork[workId] = pthread_cond_signal (&fifoFull)) != 0)       /* let a producer know that a value has been
                                                                                                            retrieved */
     { errno = statusWork[workId];                                                             /* save error in errno */
       perror ("error on signaling in fifoFull");
       statusWork[workId] = EXIT_FAILURE;
       pthread_exit (&statusWork[workId]);
     }

  if ((statusWork[workId] = pthread_mutex_unlock (&accessCR)) != 0)                                   /* exit monitor */
     { errno = statusWork[workId];                                                             /* save error in errno */
       perror ("error on exiting monitor(CF)");
       statusWork[workId] = EXIT_FAILURE;
       pthread_exit (&statusWork[workId]);
     }

  return val;
}
