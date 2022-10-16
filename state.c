#include "state.h"
#include "vector.h"

const char* washbayStateToString(wash_bay_state_t state) {
  static const char* WASH_BAY_STATE_STR[] = {
      FOREACH_WASH_BAY_STATE(GENERATE_STRING)};
  return WASH_BAY_STATE_STR[state];
}

const char* customerStateToString(customer_state_t state) {
  static const char* CUSTOMER_STATE_STR[] = {
      FOREACH_CUSTOMER_STATE(GENERATE_STRING)};
  return CUSTOMER_STATE_STR[state];
}
const char* employeeStateToString(employee_state_t state) {
  static const char* EMPLOYEE_STATE_STR[] = {
      FOREACH_EMPLOYEE_STATE(GENERATE_STRING)};
  return EMPLOYEE_STATE_STR[state];
}

void state_init(state_t* state, size_t num_employees, size_t num_customers,
                size_t num_wash_bays) {
  state->carwash_opened = 0;
  state->free_vacuum_stations = 2;
  vector_init(maintenance_event_t, &state->maintenance_log);
  vector_init_with_capacity(wash_bay_t, &state->wash_bays, num_wash_bays);
  vector_init_with_capacity(customer_t, &state->customers, num_customers);
  vector_init_with_capacity(employee_t, &state->employees, num_employees);
  for (size_t i = 0; i < num_wash_bays; ++i) {
    wash_bay_t* wash_bay = vector_emplace_back(wash_bay_t, &state->wash_bays);
    wash_bay->id = i;
    wash_bay->state = WASH_BAY_INITIAL;
    wash_bay->customer = -1;
  }
  for (size_t i = 0; i < num_customers; ++i) {
    customer_t* customer = vector_emplace_back(customer_t, &state->customers);
    customer->id = i;
    customer->state = CUSTOMER_INITIAL;
    customer->wash_bay = -1;
  }
  for (size_t i = 0; i < num_employees; ++i) {
    employee_t* employee = vector_emplace_back(employee_t, &state->employees);
    employee->id = i;
    employee->state = EMPLOYEE_INITIAL;
  }
}

void state_destroy(state_t* state) {
  vector_free(wash_bay_t, &state->wash_bays);
  vector_free(customer_t, &state->customers);
  vector_free(employee_t, &state->employees);
  vector_free(maintenance_event_t, &state->maintenance_log);
}

void updateMaintenanceLog(vector_t* maintenance_log, size_t employee) {
  maintenance_event_t* it = vector_end(maintenance_event_t, maintenance_log);
  maintenance_event_t* start =
      vector_start(maintenance_event_t, maintenance_log);
  while (it-- != start) {
    if (it->type == MAINTENANCE_CHECKING && it->employee == employee) {
      it->type = MAINTENANCE_DECREASE;
      return;
    } else if (it->type == MAINTENANCE_NOTHING_TO_DO &&
               it->employee == employee) {
      return;
    }
  }
}

void state_transition(state_t* state, event_t* event) {
  wash_bay_t* wash_bay;
  customer_t* customer;
  employee_t* employee;
  maintenance_event_t* maintenance_event;
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
      wash_bay = vector_get(wash_bay_t, &state->wash_bays, event->wash_bay);
      customer->wash_bay = wash_bay->id;
      wash_bay->customer = customer->id;
      break;
    case CUSTOMER_LEAVES_WASH_BAY:
      vector_get(wash_bay_t, &state->wash_bays, customer->wash_bay)->customer =
          -1;
      maintenance_event =
          vector_emplace_back(maintenance_event_t, &state->maintenance_log);
      maintenance_event->type = MAINTENANCE_INCREASE;
      maintenance_event->employee = -1;
      break;
    case CUSTOMER_GOES_TO_VACUUM_STATION:
      state->free_vacuum_stations--;
      break;
    case CUSTOMER_LEFT_VACUUM_STATION:
      state->free_vacuum_stations++;
      break;
    default:
      break;
    }
    break;
  case EVENT_EMPLOYEE:
    employee = vector_get(employee_t, &state->employees, event->employee);
    employee->state = event->employee_state;
    if (event->employee_state == EMPLOYEE_CHECKING_WASH_BAYS) {
      updateMaintenanceLog(&state->maintenance_log, employee->id);
    }
    maintenance_event =
        vector_emplace_back(maintenance_event_t, &state->maintenance_log);
    maintenance_event->employee = employee->id;
    maintenance_event->line_num = event->line_num;
    switch (event->employee_state) {
    case EMPLOYEE_CHECKING_WASH_BAYS:
      maintenance_event->type = MAINTENANCE_CHECKING;
      break;
    case EMPLOYEE_HAS_NOTHING_TO_DO:
      maintenance_event->type = MAINTENANCE_NOTHING_TO_DO;
      break;
    case EMPLOYEE_INITIAL: // Impossible
      break;
    }
    break;
  }
}
