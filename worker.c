#include "worker.h"
#include "pthread.h"
#include "sys/wait.h"
#include "unistd.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

job* queue_get_next_job(job_queue* queue) {
  pthread_mutex_lock(&queue->lock);
  if (queue->next_job >= queue->size) {
    pthread_mutex_unlock(&queue->lock);
    return NULL;
  } else {
    job* job = queue->jobs + (queue->next_job++);
    pthread_mutex_lock(&job->lock);
    assert(job->state == JOB_NEW);
    job->state = JOB_RUNNING;
    pthread_mutex_unlock(&queue->lock);
    return job;
  }
}

#define FILE_NAME_LIMIT 32

void runJob(worker* worker, job* job) {
  job->result = malloc(sizeof(result));
  result* result = job->result;
  result->result = PASS;
  pid_t pid = fork();
  if (pid == 0) {
    char logFile[FILE_NAME_LIMIT];
    snprintf(logFile, FILE_NAME_LIMIT, "logs/%10zu.log", job->number);
    freopen(logFile, "w", stdout);
    freopen(logFile, "w", stderr);
    if (execv(job->args[0], job->args)) {
      fprintf(stderr, "[ \x1B[1;31mERROR\x1B[0m ] Could not execute %s.\n",
              job->args[0]);
      exit(-1);
    }
  }
  waitpid(pid, &result->exit_status, 0);
  if(!WIFEXITED(result->exit_status) && WEXITSTATUS(result->exit_status)){
      result->result = FAIL;
  }
}

void printResult(job* job, int verbose) {
  result* result = job->result;
  if (WIFEXITED(result->exit_status) && WEXITSTATUS(result->exit_status) == 0) {
    if (verbose) {
      printf("[ \x1B[1;32mOK\x1B[0m ] %zu exited normally.\n", job->number);
    }
  } else {
    printf("[ \x1B[1;31mERROR\x1B[0m ] %zu exited with exit code %i.\n",
           job->number, WEXITSTATUS(result->exit_status));
  }
}

void* work(worker* worker) {
  while (1) {
    job* job = queue_get_next_job(worker->queue);
    if (!job)
      return 0;
    runJob(worker, job);
    job->state = JOB_FINISHED;
    pthread_cond_broadcast(&job->state_changed);
    pthread_mutex_unlock(&job->lock);
  }
}
