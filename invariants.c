#include "invariants.h"
#include "error.h"
#include "state.h"
#include "vector.h"
#include <stdio.h>

#define ENUM_LENGTH 32
void checkInvalidTransition(state_t* state, event_t* event, vector_t* errors) {
  char* message;
  error_t* error;

  const char* format_str;
  const char* from;
  const char* to;
  size_t id;

  wash_bay_t* wash_bay;
  customer_t* customer;
  employee_t* employee;

  switch (event->type) {
  case EVENT_OPEN:
    if (state->carwash_opened) {
      asprintf(&message, "Carwash is already open.\n");
      goto error;
    }
    break;
  case EVENT_CLOSE:
    if (!state->carwash_opened) {
      asprintf(&message, "Carwash is already closed.\n");
      goto error;
    }
    break;
  case EVENT_WASH_BAY:
    wash_bay = vector_get(wash_bay_t, &state->wash_bays, event->wash_bay);
    from = washbayStateToString(wash_bay->state);
    to = washbayStateToString(event->wash_bay_state);
    id = wash_bay->id;
    format_str = "Washbay %zu: from %s to %s.\n";
    switch (event->wash_bay_state) {
    case WASH_BAY_WAITING_FOR_CUSTOMER:
      if (wash_bay->state != WASH_BAY_INITIAL &&
          wash_bay->state != WASH_BAY_READY_FOR_NEW_CUSTOMER)
        goto from_to_error;
      break;
    case WASH_BAY_WASHING_BEFORE_PROGRAM_SELECTED:
      goto from_to_error;
    case WASH_BAY_WASHING_CAR:
      if (wash_bay->state != WASH_BAY_WAITING_FOR_CUSTOMER)
        goto from_to_error;
      break;
    case WASH_BAY_FINISHED_WASHING:
      if (wash_bay->state != WASH_BAY_WASHING_CAR)
        goto from_to_error;
      break;
    case WASH_BAY_READY_FOR_NEW_CUSTOMER:
      if (wash_bay->state != WASH_BAY_FINISHED_WASHING)
        goto from_to_error;
      break;
    case WASH_BAY_INITIAL: // Impossible
      break;
    }
    break;
  case EVENT_CUSTOMER:
    customer = vector_get(customer_t, &state->customers, event->customer);
    from = customerStateToString(customer->state);
    to = customerStateToString(event->customer_state);
    id = customer->id;
    format_str = "Customer %zu: from %s to %s.\n";
    switch (event->customer_state) {
    case CUSTOMER_LOOKING_FOR_FREE_WASH_BAY:
      if (customer->state != CUSTOMER_INITIAL)
        goto from_to_error;
      break;
    case CUSTOMER_FOUND_FREE_WASH_BAY:
      if (customer->state != CUSTOMER_LOOKING_FOR_FREE_WASH_BAY)
        goto from_to_error;
      break;
    case CUSTOMER_TRIES_TO_DRIVE_IN_USED_WASH_BAY:
      goto from_to_error;
    case CUSTOMER_SELECT_WASHING_PROGRAM:
      if (customer->state != CUSTOMER_FOUND_FREE_WASH_BAY)
        goto from_to_error;
      break;
    case CUSTOMER_NOT_OWN_CAR_IN_WASH_BAY:
      goto from_to_error;
    case CUSTOMER_LEAVES_WASH_BAY:
      if (customer->state != CUSTOMER_SELECT_WASHING_PROGRAM)
        goto from_to_error;
      break;
    case CUSTOMER_GOES_TO_VACUUM_STATION:
      if (customer->state != CUSTOMER_LEAVES_WASH_BAY)
        goto from_to_error;
      break;
    case CUSTOMER_VACUUMS_CAR:
      if (customer->state != CUSTOMER_GOES_TO_VACUUM_STATION)
        goto from_to_error;
      break;
    case CUSTOMER_LEFT_VACUUM_STATION:
      if (customer->state != CUSTOMER_VACUUMS_CAR)
        goto from_to_error;
      break;
    case CUSTOMER_INITIAL: // Impossible
      break;
    }
    break;
  case EVENT_EMPLOYEE:
    employee = vector_get(employee_t, &state->employees, event->employee);
    from = employeeStateToString(employee->state);
    to = employeeStateToString(event->employee_state);
    id = employee->id;
    format_str = "Employee %zu: from %s to %s.\n";
    switch (event->employee_state) {
    case EMPLOYEE_CHECKING_WASH_BAYS:
      break;
    case EMPLOYEE_HAS_NOTHING_TO_DO:
      if (employee->state != EMPLOYEE_CHECKING_WASH_BAYS)
        goto from_to_error;
      break;
    case EMPLOYEE_INITIAL: // Impossible
      break;
    }
  }
  return;
from_to_error:
  asprintf(&message, format_str, id, from, to);
  goto error;
error:
  error = vector_emplace_back(error_t, errors);
  error->message = message;
  error->line_num = event->line_num;
  error->type = ERROR_INVALID_TRANSITION;
}

void checkSynchronization(state_t* state, event_t* event, vector_t* errors) {
  char* message;
  error_t* error;
  error_type_t type;

  wash_bay_t* wash_bay;
  customer_t* customer;
  employee_t* employee;

  wash_bay_t* wash_bay_end;
  customer_t* customer_end;
  employee_t* employee_end;

  switch (event->type) {
  case EVENT_OPEN:
    wash_bay = vector_start(wash_bay_t, &state->wash_bays);
    wash_bay_end = vector_end(wash_bay_t, &state->wash_bays);
    for (; wash_bay != wash_bay_end; ++wash_bay) {
      if (wash_bay->state != WASH_BAY_INITIAL) {
        asprintf(&message, "Washbay %zu is already in state %s.\n",
                 wash_bay->id, washbayStateToString(wash_bay->state));
        goto open_error;
      }
    }
    customer = vector_start(customer_t, &state->customers);
    customer_end = vector_end(customer_t, &state->customers);
    for (; customer != customer_end; ++customer) {
      if (customer->state != CUSTOMER_INITIAL) {
        asprintf(&message, "Customer %zu is already in state %s.\n",
                 customer->id, customerStateToString(customer->state));
        goto open_error;
      }
    }
    employee = vector_start(employee_t, &state->employees);
    employee_end = vector_end(employee_t, &state->employees);
    for (; employee != employee_end; ++employee) {
      if (employee->state != EMPLOYEE_INITIAL) {
        asprintf(&message, "Employee %zu is already in state %s.\n",
                 employee->id, employeeStateToString(employee->state));
        goto open_error;
      }
    }
    break;
  case EVENT_WASH_BAY:
    wash_bay = vector_get(wash_bay_t, &state->wash_bays, event->wash_bay);
    switch (event->wash_bay_state) {
    case WASH_BAY_WAITING_FOR_CUSTOMER:
    case WASH_BAY_READY_FOR_NEW_CUSTOMER:
      if (wash_bay->customer != -1) {
        asprintf(&message,
                 "Washbay %zu in state %s expects that no customer in the bay, "
                 "but was customer %zu was in there.\n",
                 wash_bay->id, washbayStateToString(event->wash_bay_state),
                 wash_bay->customer);
        goto sync_error;
      }
      break;
    case WASH_BAY_WASHING_CAR:
    case WASH_BAY_FINISHED_WASHING:
      customer = vector_get(customer_t, &state->customers, event->customer);
      if (wash_bay->customer != event->customer) {
        asprintf(&message,
                 "Washbay %zu in state %s expects that customer %zu is the "
                 "washbay, but was customer %zu.\n",
                 wash_bay->id, washbayStateToString(event->wash_bay_state),
                 event->customer, wash_bay->customer);
        goto sync_error;
      }
      if (customer->state != CUSTOMER_SELECT_WASHING_PROGRAM) {
        asprintf(&message,
                 "Washbay %zu in state %s expects that customer %zu is in "
                 "state %s but was %s.\n",
                 wash_bay->id, washbayStateToString(event->wash_bay_state),
                 customer->id,
                 customerStateToString(CUSTOMER_SELECT_WASHING_PROGRAM),
                 customerStateToString(customer->state));
        goto sync_error;
      }
      break;
    case WASH_BAY_INITIAL:
    case WASH_BAY_WASHING_BEFORE_PROGRAM_SELECTED:
      break;
    }
    break;
  case EVENT_CUSTOMER:
    customer = vector_get(customer_t, &state->customers, event->customer);
    switch (event->customer_state) {
    case CUSTOMER_FOUND_FREE_WASH_BAY:
      wash_bay = vector_get(wash_bay_t, &state->wash_bays, event->wash_bay);
      if (wash_bay->customer != -1) {
        asprintf(&message,
                 "Customer %zu in state %s expects that washbay %zu is not "
                 "occupied, but was occupied by customer %zu.\n",
                 customer->id, customerStateToString(event->customer_state),
                 wash_bay->id, wash_bay->customer);
        goto sync_error;
      }
      if (wash_bay->state != WASH_BAY_WAITING_FOR_CUSTOMER &&
          wash_bay->state != WASH_BAY_INITIAL) {
  asprintf(&message,
           "Customer %zu in state %s expects that washbay %zu is in "
           "state %s or %s but was in state %s.\n",
           customer->id, customerStateToString(event->customer_state),
           wash_bay->id, washbayStateToString(WASH_BAY_WAITING_FOR_CUSTOMER),
           washbayStateToString(WASH_BAY_INITIAL),
           washbayStateToString(wash_bay->state));
        goto sync_error;
      }
      // Fall through
    case CUSTOMER_LOOKING_FOR_FREE_WASH_BAY:
      if (customer->wash_bay != -1) {
        asprintf(&message,
                 "Customer %zu in state %s expects not to be in a "
                 "washbay but was in washbay %zu.\n",
                 customer->id, customerStateToString(event->customer_state),
                 customer->wash_bay);
        goto sync_error;
      }
      break;
    case CUSTOMER_SELECT_WASHING_PROGRAM:
      wash_bay = vector_get(wash_bay_t, &state->wash_bays, customer->wash_bay);
      if (wash_bay->state != WASH_BAY_WAITING_FOR_CUSTOMER && wash_bay->state != WASH_BAY_READY_FOR_NEW_CUSTOMER && wash_bay->state != WASH_BAY_INITIAL) {
        asprintf(&message,
                 "Customer %zu in state %s expects that washbay %zu is in "
                 "state %s, %s or %s but was in state %s.\n",
                 customer->id, customerStateToString(event->customer_state),
                 wash_bay->id,
                 washbayStateToString(WASH_BAY_WAITING_FOR_CUSTOMER),
                 washbayStateToString(WASH_BAY_READY_FOR_NEW_CUSTOMER),
                 washbayStateToString(WASH_BAY_INITIAL),
                 washbayStateToString(wash_bay->state));
        goto sync_error;
      }
      if (wash_bay->customer != customer->id) {
        asprintf(&message,
                 "Customer %zu in state %s expects to be in washbay %zu but "
                 "customer %zu is in there.\n",
                 customer->id, customerStateToString(event->customer_state),
                 wash_bay->id, wash_bay->customer);
        goto sync_error;
      }
      break;
    case CUSTOMER_LEAVES_WASH_BAY:
      wash_bay = vector_get(wash_bay_t, &state->wash_bays, customer->wash_bay);
      if (wash_bay->state != WASH_BAY_FINISHED_WASHING) {
        asprintf(&message,
                 "Customer %zu in state %s expects that washbay %zu is in "
                 "state %s but was in state %s.\n",
                 customer->id, customerStateToString(event->customer_state),
                 wash_bay->id, washbayStateToString(WASH_BAY_FINISHED_WASHING),
                 washbayStateToString(wash_bay->state));
        goto sync_error;
      }
      break;
    case CUSTOMER_GOES_TO_VACUUM_STATION:
      if (state->free_vacuum_stations == 0) {
        asprintf(&message,
                 "Customer %zu in state %s expects that a vacuum station is "
                 "available.\n",
                 customer->id, customerStateToString(event->customer_state));
        goto sync_error;
      }
      break;
    case CUSTOMER_NOT_OWN_CAR_IN_WASH_BAY:
    case CUSTOMER_TRIES_TO_DRIVE_IN_USED_WASH_BAY:
    case CUSTOMER_VACUUMS_CAR:
    case CUSTOMER_LEFT_VACUUM_STATION:
    case CUSTOMER_INITIAL:
      break;
    }
  case EVENT_EMPLOYEE:
  case EVENT_CLOSE:
    break;
  }
  return;
open_error:
  type = ERROR_ACTIVITY_BEFORE_OPEN;
  goto error;
sync_error:
  type = ERROR_SYNCHRONIZATION;
  goto error;
error:
  error = vector_emplace_back(error_t, errors);
  error->type = type;
  error->message = message;
  error->line_num = event->line_num;
}

void checkEvent(state_t* state, event_t* event, vector_t* errors) {
  checkInvalidTransition(state, event, errors);
  if (!vector_is_empty(error_t, errors)) {
    return;
  }
  checkSynchronization(state, event, errors);
}

void checkFinalState(state_t* state, vector_t* errors) {
  if (state->carwash_opened != 0) {
    error_t* error = vector_emplace_back(error_t, errors);
    error->type = ERROR_CARWASH_NOT_CLOSED;
    error->message = NULL;
    error->line_num = -1;
  }
}
