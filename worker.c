#include "worker.h"
#include "error.h"
#include "invariants.h"
#include "vector.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

job_t* queue_get_next_job(job_queue_t* queue) {
  pthread_mutex_lock(&queue->lock);
  if (queue->next_job >= queue->size) {
    pthread_mutex_unlock(&queue->lock);
    return NULL;
  } else {
    job_t* job = queue->jobs + (queue->next_job++);
    pthread_mutex_lock(&job->lock);
    assert(job->state == JOB_NEW);
    job->state = JOB_RUNNING;
    pthread_mutex_unlock(&queue->lock);
    return job;
  }
}

int substr(char** line, char* substr) {
  size_t substrLen = strlen(substr);
  if (!strncmp(*line, substr, substrLen)) {
    *line += substrLen;
    return 1;
  } else
    return 0;
}

void parseLine(char* line, event_t* event, vector_t* errors, size_t line_num) {
  char* lineOrig = line;
  error_t* error;
  if (!strcmp(line, "CAR WASH PARK OPENED!\n")) {
    event->type = EVENT_OPEN;
  } else if (!strcmp(line, "CAR WASH PARK CLOSED!\n")) {
    event->type = EVENT_CLOSE;
  } else if (substr(&line, "WashBay ")) {
    event->type = EVENT_WASH_BAY;
    event->wash_bay = strtoll(line, &line, 10);
    if (!strcmp(line, " : waiting for new customers.\n")) {
      event->wash_bay_state = WASH_BAY_WAITING_FOR_CUSTOMER;
    } else if (!strcmp(line, " : washing before wash program selected.\n")) {
      event->wash_bay_state = WASH_BAY_WASHING_BEFORE_PROGRAM_SELECTED;
    } else if (substr(&line, " : washing car of customer ")) {
      event->wash_bay_state = WASH_BAY_WASHING_CAR;
      event->customer = strtoll(line, &line, 10);
      if (strcmp(line, ".\n"))
        goto err;
    } else if (substr(&line, " : the car of customer ")) {
      event->wash_bay_state = WASH_BAY_FINISHED_WASHING;
      event->customer = strtoll(line, &line, 10);
      if (strcmp(line, " washed, can be picked up.\n"))
        goto err;
    } else if (!strcmp(line, " is ready for new customers.\n")) {
      event->wash_bay_state = WASH_BAY_READY_FOR_NEW_CUSTOMER;
    } else {
      goto err;
    }
  } else if (substr(&line, "Customer ")) {
    event->type = EVENT_CUSTOMER;
    event->customer = strtoll(line, &line, 10);
    if (!strcmp(line, " goes to the vacuum station...\n")) {
      event->customer_state = CUSTOMER_GOES_TO_VACUUM_STATION;
    } else if (!strcmp(line, " vacuums his car...\n")) {
      event->customer_state = CUSTOMER_VACUUMS_CAR;
    } else if (!strcmp(line, " left vacuum station...\n")) {
      event->customer_state = CUSTOMER_LEFT_VACUUM_STATION;
    } else if (!strcmp(line, " is looking for a free wash bay.\n")) {
      event->customer_state = CUSTOMER_LOOKING_FOR_FREE_WASH_BAY;
    } else if (substr(&line, " found a free wash bay ")) {
      event->customer_state = CUSTOMER_FOUND_FREE_WASH_BAY;
      event->wash_bay = strtoll(line, &line, 10);
      if (strcmp(line, "\n"))
        goto err;
    } else if (!strcmp(line, " tries to drive into a used wash bay :o\n")) {
      event->customer_state = CUSTOMER_TRIES_TO_DRIVE_IN_USED_WASH_BAY;
    } else if (!strcmp(line, " didn't find his car in the wash bay :o.\n")) {
      event->customer_state = CUSTOMER_NOT_OWN_CAR_IN_WASH_BAY;
    } else if (!strcmp(line, " leaves the wash bay...\n")) {
      event->customer_state = CUSTOMER_LEAVES_WASH_BAY;
    } else if (!strcmp(line, " selected a washing program and can have a "
                             "coffee break now.\n")) {
      event->customer_state = CUSTOMER_SELECT_WASHING_PROGRAM;
    } else {
      goto err;
    }
  } else if (substr(&line, "Employee ")) {
    event->type = EVENT_EMPLOYEE;
    event->employee = strtoll(line, &line, 10);
    if (!strcmp(line, " is checking wash bays...\n")) {
      event->employee_state = EMPLOYEE_CHECKING_WASH_BAYS;
    } else if (!strcmp(line, " has nothing to do...\n")) {
      event->employee_state = EMPLOYEE_HAS_NOTHING_TO_DO;
    } else {
      goto err;
    }
  } else {
  err:
    error = vector_emplace_back(error_t, errors);
    error->type = ERROR_PARSING;
    error->message = lineOrig;
    error->line_num = line_num;
  }
}

void stateTransition(state_t* state, event_t* event) {
  wash_bay_t* wash_bay;
  customer_t* customer;
  employee_t* employee;
  switch (event->type) {
  case EVENT_OPEN:
    state->carwash_opened = 1;
    break;
  case EVENT_CLOSE:
    state->carwash_opened = 0;
    break;
  case EVENT_WASH_BAY:
    wash_bay = vector_get(wash_bay_t, &state->wash_bays, event->wash_bay);
    wash_bay->state = event->wash_bay_state;
    switch (event->wash_bay_state) {
    case WASH_BAY_FINISHED_WASHING:
    case WASH_BAY_WASHING_CAR:
      wash_bay->customer = event->customer;
      break;
    default:
      break;
    }
    break;
  case EVENT_CUSTOMER:
    customer = vector_get(customer_t, &state->customers, event->customer);
    customer->state = event->customer_state;
    switch (event->customer_state) {
    case CUSTOMER_FOUND_FREE_WASH_BAY:
      customer->wash_bay = event->wash_bay;
      break;
    case CUSTOMER_LEAVES_WASH_BAY:
      vector_get(wash_bay_t, &state->wash_bays, customer->wash_bay)->customer =
          -1;
      break;
    default:
      break;
    }
    break;
  case EVENT_EMPLOYEE:
    employee = vector_get(employee_t, &state->employees, event->employee);
    employee->state = event->employee_state;
    break;
  }
}

#define FILE_NAME_LIMIT 32

void checkFile(FILE* file, job_t* job) {
  char* line = NULL;
  size_t len;
  ssize_t nread;
  state_t state;
  vector_t* errors = &job->result->errors;
  state_init(&state, atoll(job->args[1]), atoll(job->args[2]),
             atoll(job->args[3]));
  char logFile[FILE_NAME_LIMIT];
  snprintf(logFile, FILE_NAME_LIMIT, "logs/%06zu.log", job->number);
  FILE* log = fopen(logFile, "w");
  size_t line_num = 0;

  while ((nread = getline(&line, &len, file)) != -1) {
    event_t event;
    line_num++;
    fputs(line, log);
    parseLine(line, &event, errors, line_num);
    if (!vector_is_empty(error_t, errors)) {
      job->result->result = FAIL;
      line = NULL;
      break;
    }
    checkEvent(&state, &event, errors, line_num);
    if (!vector_is_empty(error_t, errors)) {
      job->result->result = FAIL;
      break;
    }
    stateTransition(&state, &event);
  }
  while ((nread = getline(&line, &len, file)) != -1) {
    fputs(line, log);
  }
  checkFinalState(&state, errors);
  if (!vector_is_empty(error_t, errors)) {
    job->result->result = FAIL;
  }
  free(line);
  state_destroy(&state);
  fclose(file);
  fclose(log);
}

void runJob(worker_t* worker, job_t* job) {
  job->result = malloc(sizeof(result_t));
  result_t* result = job->result;
  result->result = PASS;
  vector_init(error_t, &job->result->errors);
  int fd[2];
  assert(pipe(fd) == 0);
  pid_t pid = fork();
  if (pid == 0) {
    close(fd[0]);
    char errFile[FILE_NAME_LIMIT];
    snprintf(errFile, FILE_NAME_LIMIT, "logs/%06zu.err", job->number);
    freopen(errFile, "w", stderr);
    close(1);
    dup(fd[1]);
    close(0);
    if (execv(job->args[0], job->args)) {
      fprintf(stderr, "[ \x1B[1;31mERROR\x1B[0m ] Could not execute %s.\n",
              job->args[0]);
      exit(-1);
    }
  }
  close(fd[1]);
  FILE* file = fdopen(fd[0], "r");
  assert(file != NULL);
  checkFile(file, job);
  waitpid(pid, &result->exit_status, 0);
  if (!WIFEXITED(result->exit_status) && WEXITSTATUS(result->exit_status)) {
    result->result = FAIL;
  }
}

void printResult(job_t* job, int verbose) {
  result_t* result = job->result;
  error_t* it = vector_start(error_t, &result->errors);
  error_t* end = vector_end(error_t, &result->errors);
  for (; it != end; ++it) {
    printError(it, job->number);
    error_destroy(it);
  }
  if (WIFEXITED(result->exit_status) && WEXITSTATUS(result->exit_status) == 0) {
    if (verbose) {
      printf("[ \x1B[1;32mOK\x1B[0m ] %zu exited normally.\n", job->number);
    }
  } else {
    printf("[ \x1B[1;31mERROR\x1B[0m ] %zu exited with exit code %i.\n",
           job->number, WEXITSTATUS(result->exit_status));
  }
}

void* work(worker_t* worker) {
  while (1) {
    job_t* job = queue_get_next_job(worker->queue);
    if (!job)
      return 0;
    runJob(worker, job);
    job->state = JOB_FINISHED;
    pthread_cond_broadcast(&job->state_changed);
    pthread_mutex_unlock(&job->lock);
  }
}
