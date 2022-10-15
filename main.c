#include "pthread.h"
#include "vector.h"
#include "worker.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define DEFAULT_COMMAND "./carwash"
#define DEFAULT_NUM 10
#define DEFAULT_JOBS 4

typedef struct {
  char* command;
  int num;
  int workers;
  char* args;
  int verbose;
} cli_args;

void printHelp() {
  printf("Usage: slp-a2-test [Options]\n"
         "Options:\n"
         "  --args ARGS                 the executable will be only tested "
         "with ARGS as arguments\n"
         "  -c COMMAND                  COMMAND is the executable that will be "
         "tested\n"
         "  -j N                        execute N tests in parallel\n"
         "  -n N                        execute a test scenario N times\n"
         "  -v, --verbose               prints more details\n"
         "  -h, --help                  prints this help message\n"
         "  --version                   prints the version\n");
  exit(0);
}

void printVersion() {
  printf("slp-a2-test 1.0.0\n");
  exit(0);
}

#define VERSION -2
#define ARGS -3

void parseCliArgs(int argc, char* argv[], cli_args* args) {
  int c;
  args->command = DEFAULT_COMMAND;
  args->num = DEFAULT_NUM;
  args->workers = DEFAULT_JOBS;
  args->args = NULL;
  args->verbose = 0;
  while (1) {
    int option_index = 0;
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, VERSION},
        {"args", required_argument, NULL, ARGS},
        {"verbose", no_argument, NULL, 'v'},
        {0, 0, 0, 0}};
    c = getopt_long(argc, argv, "hc:n:j:v", long_options, &option_index);
    if (c == -1)
      break;

    int n;
    switch (c) {
    case 'h':
      printHelp();
      break;
    case VERSION:
      printVersion();
      break;
    case ARGS:
      args->args = optarg;
      break;
    case 'c':
      args->command = optarg;
      break;
    case 'n':
      n = atoi(optarg);
      if (n <= 0) {
        fprintf(stderr, "-n: Please enter a number greater than 0\n");
        exit(64);
      }
      args->num = n;
      break;
    case 'j':
      n = atoi(optarg);
      if (n <= 0) {
        fprintf(stderr, "-j: Please enter a number greater than 0\n");
        exit(64);
      }
      args->workers = n;
      break;
    case 'v':
      args->verbose = 1;
      break;
    default:
      fprintf(stderr, "\x1B[1;31minternal error\x1B[0m\n");
      exit(-1);
    }
  }
}

void parseArgs(char* command, char* args, char* (*args_array)[5]) {
  char* arg[3];
  if (!args) {
    arg[0] = "10";
    arg[1] = "100";
    arg[2] = "10";
  } else {
    for (int i = 0; i < 3; ++i) {
      arg[i] = strsep(&args, " ");
    }
  }
  (*args_array)[0] = command;
  (*args_array)[1] = arg[0];
  (*args_array)[2] = arg[1];
  (*args_array)[3] = arg[2];
  (*args_array)[4] = NULL;
}

void init_queue(job_queue_t* queue, cli_args* args) {
  queue->size = args->num;
  queue->next_job = 0;
  queue->jobs = malloc(queue->size * sizeof(job_t));
  pthread_mutex_init(&queue->lock, NULL);
  char* args_array[5];
  parseArgs(args->command, args->args, &args_array);
  for (size_t i = 0; i < args->num; ++i) {
    job_t* job = queue->jobs + i;
    memcpy(job->args, args_array, 5 * sizeof(char*));
    job->number = i;
    job->state = JOB_NEW;
    pthread_mutex_init(&job->lock, NULL);
    pthread_cond_init(&job->state_changed, NULL);
  }
}

void printProgress(size_t finished, size_t total, size_t failed) {
  printf("[ %zu/%zu", finished, total);
  if (failed)
    printf("; \x1B[1;31m%zu failed\x1B[0m", failed);
  printf(" ]");
  fflush(stdout);
}

void removeProgress() { printf("\x1B[2K\x1B[G"); }

int main(int argc, char* argv[]) {
  cli_args args;
  parseCliArgs(argc, argv, &args);
  if (args.verbose) {
    printf("Testing \x1B[94m%s\x1B[0m\n", args.command);
  }
  job_queue_t* queue = malloc(sizeof(job_queue_t));
  init_queue(queue, &args);
  worker_t* workers = malloc(args.workers * sizeof(worker_t));
  for (size_t i = 0; i < args.workers; ++i) {
    worker_t* worker = workers + i;
    worker->id = i;
    worker->queue = queue;
    pthread_create(&worker->thread, NULL, ((void* (*) (void*) ) work), worker);
  }

  mkdir("logs", 0755);

  printProgress(0, queue->size, 0);
  size_t failed = 0;
  for (size_t i = 0; i < queue->size; i++) {
    job_t* job = queue->jobs + i;
    pthread_mutex_lock(&job->lock);
    while (job->state != JOB_FINISHED) {
      pthread_cond_wait(&job->state_changed, &job->lock);
    }
    if (job->result->result == FAIL)
      failed++;
    removeProgress();
    printResult(job, args.verbose);
    printProgress(job->number + 1, queue->size, failed);
    pthread_mutex_unlock(&job->lock);
  }

  for (size_t i = 0; i < args.workers; ++i) {
    pthread_join(workers[i].thread, NULL);
  }

  removeProgress();
  fflush(stdout);
  if (failed == 0) {
    printf("Ran %zu tests with \x1B[1;32m0\x1B[0m failures.\n", queue->size);
    return EXIT_SUCCESS;
  } else {
    printf("Ran %zu tests with \x1B[1;31m%zu\x1B[0m failures.\n", queue->size,
           failed);
  }
}
