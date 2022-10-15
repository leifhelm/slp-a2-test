#ifndef ERROR_H_
#define ERROR_H_

typedef enum {
  ERROR_ACTIVITY_BEFORE_OPEN,
  ERROR_PARSING,
  ERROR_CARWASH_NOT_CLOSED
} error_type_t;

typedef struct {
  error_type_t type;
  char* line;
} error_t;

#endif // ERROR_H_
