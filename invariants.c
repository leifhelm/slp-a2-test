#include "invariants.h"
#include "error.h"
#include "vector.h"

void checkEvent(state_t* state, event_t* event, vector_t* errors) {}

void checkFinalState(state_t* state, vector_t* errors) {
  if (state->carwash_opened != 0) {
    error_t* error = vector_emplace_back(error_t, errors);
    error->type = ERROR_CARWASH_NOT_CLOSED;
  }
}
