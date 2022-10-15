#ifndef INVARIANTS_H_
#define INVARIANTS_H_

#include "state.h"
#include "vector.h"

void checkEvent(state_t* state, event_t* event, vector_t* errors, size_t line_num);
void checkFinalState(state_t* state, vector_t* errors);

#endif // INVARIANTS_H_
