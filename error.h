#ifndef ERROR_H_
#define ERROR_H_

#include <stddef.h>

#define ENUM_AND_STRING_ENUM(ENUM, STRING) ENUM,
#define ENUM_AND_STRING_STRING(ENUM, STRING) STRING,

#define FOREACH_ERROR_TYPE(F)                                                  \
  F(ERROR_ACTIVITY_BEFORE_OPEN, "activity before carwash is open")             \
  F(ERROR_PARSING, "parsing error")                                            \
  F(ERROR_INVALID_TRANSITION, "invalid transition")                            \
  F(ERROR_CARWASH_NOT_CLOSED, "carwash is not closed in the end")

typedef enum { FOREACH_ERROR_TYPE(ENUM_AND_STRING_ENUM) } error_type_t;

typedef struct {
  error_type_t type;
  char* message;
  size_t line_num;
} error_t;

void error_destroy(error_t* error);

void printError(error_t* error, size_t job_number);

#endif // ERROR_H_
