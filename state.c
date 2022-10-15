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
}
