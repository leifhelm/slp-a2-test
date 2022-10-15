#include "error.h"
#include <stdio.h>
#include <stdlib.h>

void error_destroy(error_t* error) { free(error->message); }

const char* typeToString(error_type_t type) {
  const char* type_str[] = {FOREACH_ERROR_TYPE(ENUM_AND_STRING_STRING)};
  return type_str[type];
}

void printError(error_t* error, size_t job_number) {
  printf("[ \x1B[1;31mERROR\x1B[0m ][%zu] ", job_number);
  if (error->line_num != -1)
    printf("On line \x1B[93m%zu\x1B[0m ", error->line_num);
  printf("%s", typeToString(error->type));
  if (error->message)
    printf(": %s", error->message);
  else
    printf("\n");
}
