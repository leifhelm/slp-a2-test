#ifndef STATE_H_
#define STATE_H_

#include "vector.h"

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

#define FOREACH_WASH_BAY_STATE(F)                                              \
  F(WASH_BAY_INITIAL)                                                          \
  F(WASH_BAY_WAITING_FOR_CUSTOMER)                                             \
  F(WASH_BAY_WASHING_CAR)                                                      \
  F(WASH_BAY_FINISHED_WASHING)                                                 \
  F(WASH_BAY_READY_FOR_NEW_CUSTOMER)                                           \
  F(WASH_BAY_WASHING_BEFORE_PROGRAM_SELECTED)

typedef enum { FOREACH_WASH_BAY_STATE(GENERATE_ENUM) } wash_bay_state_t;
const char* washbayStateToString(wash_bay_state_t state);

typedef struct {
  size_t id;
  wash_bay_state_t state;
  size_t customer;
} wash_bay_t;

#define FOREACH_CUSTOMER_STATE(F)                                              \
  F(CUSTOMER_INITIAL)                                                          \
  F(CUSTOMER_LOOKING_FOR_FREE_WASH_BAY)                                        \
  F(CUSTOMER_FOUND_FREE_WASH_BAY)                                              \
  F(CUSTOMER_SELECT_WASHING_PROGRAM)                                           \
  F(CUSTOMER_LEAVES_WASH_BAY)                                                  \
  F(CUSTOMER_GOES_TO_VACUUM_STATION)                                           \
  F(CUSTOMER_VACUUMS_CAR)                                                      \
  F(CUSTOMER_LEFT_VACUUM_STATION)                                              \
  F(CUSTOMER_TRIES_TO_DRIVE_IN_USED_WASH_BAY)                                  \
  F(CUSTOMER_NOT_OWN_CAR_IN_WASH_BAY)
typedef enum { FOREACH_CUSTOMER_STATE(GENERATE_ENUM) } customer_state_t;
const char* customerStateToString(customer_state_t state);

typedef struct {
  size_t id;
  customer_state_t state;
  size_t wash_bay;
} customer_t;

#define FOREACH_EMPLOYEE_STATE(F)                                              \
  F(EMPLOYEE_INITIAL)                                                          \
  F(EMPLOYEE_CHECKING_WASH_BAYS)                                               \
  F(EMPLOYEE_HAS_NOTHING_TO_DO)

typedef enum { FOREACH_EMPLOYEE_STATE(GENERATE_ENUM) } employee_state_t;
const char* employeeStateToString(employee_state_t state);

typedef struct {
  size_t id;
  employee_state_t state;
} employee_t;

typedef struct {
  int carwash_opened;
  vector_t wash_bays;
  vector_t customers;
  vector_t employees;
} state_t;

typedef enum {
  EVENT_OPEN,
  EVENT_CLOSE,
  EVENT_WASH_BAY,
  EVENT_CUSTOMER,
  EVENT_EMPLOYEE
} event_type;

typedef struct {
  event_type type;
  wash_bay_state_t wash_bay_state;
  customer_state_t customer_state;
  employee_state_t employee_state;
  size_t wash_bay;
  size_t customer;
  size_t employee;
} event_t;

void state_init(state_t* state, size_t num_employees, size_t num_customers,
                size_t num_wash_bays);
void state_destroy(state_t* state);

#endif // STATE_H_
