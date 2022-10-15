#ifndef WORKER_H_
#define WORKER_H_

#include "vector.h"
#include <pthread.h>

typedef enum { PASS, FAIL } test_result_t;

typedef struct {
  test_result_t result;
  vector_t errors;
  int exit_status;
} result_t;

typedef enum {
  JOB_NEW,
  JOB_RUNNING,
  JOB_FINISHED,
} job_state_t;

typedef struct {
  job_state_t state;
  size_t number;
  char* args[5];
  result_t* result;
  pthread_mutex_t lock;
  pthread_cond_t state_changed;
} job_t;

typedef struct {
  pthread_mutex_t lock;
  job_t* jobs;
  size_t size;
  size_t next_job;
} job_queue_t;

typedef struct {
  job_queue_t* queue;
  size_t id;
  pthread_t thread;
} worker_t;

void printResult(job_t* job, int verbose);

void* work(worker_t* worker);

#endif // WORKER_H_
