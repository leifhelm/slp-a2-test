#ifndef STATE_H_
#define STATE_H_

#include "vector.h"

typedef enum {
  WASH_BAY_INITIAL,
  WASH_BAY_WAITING_FOR_CUSTOMER,
  WASH_BAY_WASHING_CAR,
  WASH_BAY_FINISHED_WASHING,
  WASH_BAY_READY_FOR_NEW_CUSTOMER,
  WASH_BAY_WASHING_BEFORE_PROGRAM_SELECTED,
} wash_bay_state_t;

typedef struct {
  size_t id;
  wash_bay_state_t state;
  size_t customer;
} wash_bay_t;

typedef enum {
  CUSTOMER_INITIAL,
  CUSTOMER_LOOKING_FOR_FREE_WASH_BAY,
  CUSTOMER_FOUND_FREE_WASH_BAY,
  CUSTOMER_SELECT_WASHING_PROGRAM,
  CUSTOMER_LEAVES_WASH_BAY,
  CUSTOMER_GOES_TO_VACUUM_STATION,
  CUSTOMER_VACUUMS_CAR,
  CUSTOMER_LEFT_VACUUM_STATION,
  CUSTOMER_TRIES_TO_DRIVE_IN_USED_WASH_BAY,
  CUSTOMER_NOT_OWN_CAR_IN_WASH_BAY,
} customer_state_t;

typedef struct {
  size_t id;
  customer_state_t state;
  size_t wash_bay;
} customer_t;

typedef enum {
  EMPLOYEE_INITIAL,
  EMPLOYEE_CHECKING_WASH_BAYS,
  EMPLOYEE_HAS_NOTHING_TO_DO,
} employee_state_t;

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
