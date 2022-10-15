#ifndef WORKER_H_
#define WORKER_H_

#include <pthread.h>

typedef enum { PASS, FAIL } test_result;

typedef struct {
  test_result result;
  int exit_status;
} result;

typedef enum {
  JOB_NEW,
  JOB_RUNNING,
  JOB_FINISHED,
} job_state;

typedef struct {
  job_state state;
  size_t number;
  char* args[5];
  result* result;
  pthread_mutex_t lock;
  pthread_cond_t state_changed;
} job;

typedef struct {
  pthread_mutex_t lock;
  job* jobs;
  size_t size;
  size_t next_job;
} job_queue;

typedef struct {
  job_queue* queue;
  size_t id;
  pthread_t thread;
} worker;

void printResult(job* job, int verbose);

void* work(worker* worker);

#endif // WORKER_H_
