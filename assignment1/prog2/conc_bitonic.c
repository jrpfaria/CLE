#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#include "prob_const.h"
#include "fifo.h"

#define swap(a,b) {int t = *a; *a = *b; *b = t;}

/** \brief return status on monitor initialization */
int statusInitMon;

/** \brief distributor threads return status array */
int *statusDist;

/** \brief worker threads return status array */
int *statusWork;

/** \brief number of storage positions in the data transfer region */
int nStorePos = 8;

/** \brief number of threads to execute the program with */
int nThreads = 8;

/** \brief distributor life cycle routine */
static void *distributor (void *id);

/** \brief worker life cycle routine */
static void *worker (void *id);

/** \brief execution time measurement */
static double get_delta_time(void);

/** \brief print command usage */
static void printUsage (char *cmdName);

/**
 *  \brief Main thread.
 *
 *  Its role is starting the simulation by generating the intervening entities threads (distributor and workers) and
 *  waiting for their termination.
 *
 *  \param argc number of words of the command line
 *  \param argv list of words of the command line
 *
 *  \return status of operation
 */

// Global variables
int array_length;
char* workerLives;

#define ASCENDING 1
#define DESCENDING 0
int sorting_method = ASCENDING;

// Function prototypes
void compare_and_swap(int*, int*, int);
void bitonic_merge(int*, int, int);
void bitonic_sort(int*, int, int);
void sort(int*, int, int);

int main (int argc, char *argv[])
{
  /* process command line options */

  int opt;          /* selected option */
  FILE* file;

  do
  { switch ((opt = getopt (argc, argv, "t:h:f:s")))
    { case 't': /* number of threads to be created */
                printf("Threads: %s\n", optarg);
                if (atoi (optarg) <= 0)
                   { fprintf (stderr, "%s: non positive number\n", basename (argv[0]));
                     printUsage (basename (argv[0]));
                     return EXIT_FAILURE;
                   }
                nThreads = (int) atoi (optarg);
                nStorePos = nThreads;
                if (nThreads > N)
                   { fprintf (stderr, "%s: too many threads\n", basename (argv[0]));
                     printUsage (basename (argv[0]));
                     return EXIT_FAILURE;
                   }
                break;

      case 'h': /* help mode */
                printUsage (basename (argv[0]));
                return EXIT_SUCCESS;

      case 'f': /* input file*/
                printf("File: %s\n", optarg);
                file = fopen(optarg, "r");
                if (file == NULL)
                {
                  fprintf(stderr, "Error opening file\n");
                  return EXIT_FAILURE;
                }
                break;

      case '?': /* invalid option */
                fprintf (stderr, "%s: invalidxx option\n", basename (argv[0]));
    	          printUsage (basename (argv[0]));
                return EXIT_FAILURE;

      case -1:  break;
    }
  } while (opt != -1);

  if (optind < argc)
     { fprintf (stderr, "%s: invalid format\n", basename (argv[0]));
       printUsage (basename (argv[0]));
       return EXIT_FAILURE;
     }

  if (((statusDist = malloc (sizeof (int))) == NULL) ||
      ((statusWork = malloc (nThreads * sizeof (int))) == NULL))
     { fprintf (stderr, "error on allocating space to the return status arrays of distributor / worker threads\n");
       exit (EXIT_FAILURE);
     }

  pthread_t *tIdDist,               /* distributor internal thread id array */
            *tIdWork;               /* workers internal thread id array */
  unsigned int *workers;            /* workers application defined thread id array */
  int i;                            /* counting variable */
  int *pStatus;                     /* pointer to execution status */

  /* initializing the application defined thread id arrays for the distributor and the workers */
  if (((tIdDist = malloc (sizeof (pthread_t))) == NULL) ||
      ((tIdWork = malloc (nThreads * sizeof (pthread_t))) == NULL) ||
      ((workerLives = malloc(nThreads * sizeof (char))) == NULL) ||
      ((workers = malloc (nThreads * sizeof (unsigned int))) == NULL))
     { fprintf (stderr, "error on allocating space to both internal / external distributor / worker id arrays\n");
       exit (EXIT_FAILURE);
     }
  
  // for (i = 0; i < 1; i++)
    // dist[i] = i;
  for (i = 0; i < nThreads; i++){
    workerLives[i] = 1;
    workers[i] = i;
  }
  (void) get_delta_time ();

  /* generation of intervening entities threads */
  if (pthread_create (tIdDist, NULL, distributor, (void*) file) != 0)                              /* thread distributor */
    { perror ("error on creating thread distributor");
        exit (EXIT_FAILURE);
    }
  for (i = 0; i < nThreads; i++)
    if (pthread_create (&tIdWork[i], NULL, worker, &workers[i]) != 0)                             /* thread worker */
      { perror ("error on creating thread worker");
         exit (EXIT_FAILURE);
      }

  /* waiting for the termination of the intervening entities threads */

  printf ("\nFinal report\n");
  for (i = 0; i < nThreads; i++)
  { if (pthread_join (tIdWork[i], (void *) &pStatus) != 0)                                       /* thread worker */
       { perror ("error on waiting for thread customer");
         exit (EXIT_FAILURE);
       }
    printf ("thread worker, with id %u, has terminated: ", i);
    printf ("its status was %d\n", *pStatus);
  }
  if (pthread_join (*tIdDist, (void *) &pStatus) != 0)                                       /* thread distributor */
      { perror ("error on waiting for thread distributor");
        exit (EXIT_FAILURE);
      }
  printf ("thread distributor, with id %u, has terminated: ", i);
  printf ("its status was %d\n", *pStatus);
  
  printf ("\nElapsed time = %.6f s\n", get_delta_time ());

  exit (EXIT_SUCCESS);
}

pthread_mutex_t control_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t control_cond = PTHREAD_COND_INITIALIZER;
static char control = 0;

char is_sorted(int* array, char method)
{
  if (method){
    for (int i = 0; i < array_length; i++){
      if (array[i] > array[i + 1]){
        return 0;
      }
    }
  }
  else{
    for (int i = 0; i < array_length; i++){
      if (array[i] < array[i + 1]){
        return 0;
      }
    }
  }
  return 1;
}

/**
 *  \brief Function distributor.
 *
 *  Its role is to simulate the life cycle of a distributor.
 *
 *  \param par pointer to the array to be sorted
 */
static void *distributor (void *par)
{
  FILE* input_file = (FILE*) par;

  // Get data from file  
  array_length = getw(input_file);
  int* array = malloc(array_length * sizeof(int));
  for (int i = 0; i < array_length; i++)
    array[i] = getw(input_file);

  int i, j, k,                      /* counting variable */
      load,                         /* load of the subarray */
      sort_aux = sorting_method;    /* auxiliary variable */

  for (i = 0; i <= log2(nThreads) ; i++)
  { 
    printf("Iteration %d\n", i);
    load = array_length/(nThreads>>i);
    for(k = 0; k < nThreads >> i; k++){
      printf("produced values: %p, %d, %d, %d\n", &array[k*load], load, sort_aux, i != 0);
      putVal(&array[k*load], load, sort_aux, i != 0);
      sort_aux = !sort_aux;
    }

    // Signal workers to start processing
    pthread_mutex_lock(&control_mutex);
    control = nThreads >> i;
    pthread_cond_broadcast(&control_cond);
    pthread_mutex_unlock(&control_mutex);

    pthread_mutex_lock(&control_mutex);
    while(control > 0){ /*wait for workers to finish*/
      pthread_cond_wait(&control_cond, &control_mutex);
    }
    pthread_mutex_unlock(&control_mutex);
    
    for (j = (nThreads >> i) - 1; j > log2(nThreads >> i); j--) workerLives[j] = 0; /*kill worker*/
  }

  pthread_mutex_lock(&control_mutex);
  for (i = 0; i < nThreads; i++) workerLives[i] = 0; /*make sure every worker dies*/
  control = nThreads;
  pthread_cond_broadcast(&control_cond);
  pthread_mutex_unlock(&control_mutex);

  printf("distributor has terminated\n");

  *statusDist = EXIT_SUCCESS;
  pthread_exit (statusDist);
}

/** 
 *  \brief Function worker.
 *
 *  Its role is to simulate the life cycle of a worker.
 *
 *  \param par pointer to application defined worker identification
 */

static void *worker (void *par)
{
  unsigned int id = *((unsigned int *) par);  /* worker id */
  FIFO_DATA val;

  int debug = 0;

  while(1)
  { 
    pthread_mutex_lock(&control_mutex);
    while(!control){ /*wait for distributor to send data*/
      pthread_cond_wait(&control_cond, &control_mutex);
    }
    pthread_mutex_unlock(&control_mutex);

    // Check if worker should terminate
    if (!workerLives[id]){
      break;
    }

    val = getVal(id);
    if (val.act){
      bitonic_merge(val.arr, val.num, val.opt);
    }
    else{
      bitonic_sort(val.arr, val.num, val.opt);
    }

    pthread_mutex_lock(&control_mutex);
    control--;
    pthread_mutex_unlock(&control_mutex);

    // Signal distributor that worker has finished
    pthread_mutex_lock(&control_mutex);
    pthread_cond_signal(&control_cond);
    pthread_mutex_unlock(&control_mutex);
  }

  printf ("worker %u has terminated\n", id);

  statusWork[id] = EXIT_SUCCESS;
  pthread_exit (&statusWork[id]);
}

/**
 *  \brief Get the process time that has elapsed since last call of this time.
 *
 *  \return process elapsed time
 */

static double get_delta_time(void)
{
  static struct timespec t0, t1;

  t0 = t1;
  if(clock_gettime (CLOCK_MONOTONIC, &t1) != 0)
  {
    perror ("clock_gettime");
    exit(1);
  }
  return (double) (t1.tv_sec - t0.tv_sec) + 1.0e-9 * (double) (t1.tv_nsec - t0.tv_nsec);
}

/**
 *  \brief Print command usage.
 *
 *  A message specifying how the program should be called is printed.
 *
 *  \param cmdName string with the name of the command
 */

static void printUsage (char *cmdName)
{
  fprintf (stderr, "\nSynopsis: %s [OPTIONS]\n"
           "  OPTIONS:\n"
           "  -t nThreads  --- set the number of threads to be created (default: 8)\n"
           "  -f inputFile --- set the file path to get the data from\n"
           "  -h           --- print this help\n", cmdName);
}

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