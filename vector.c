#include "vector.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void _vector_init_with_capacity(vector_t* v, size_t capacity, size_t sizeof_x) {
  v->xs = malloc(sizeof_x * capacity);
  v->capacity = capacity;
  v->size = 0;
}

void _vector_free(vector_t* v, size_t sizeof_x) { free(v->xs); }

void* _vector_get(vector_t* v, size_t index, size_t sizeof_x) {
  assert(index < v->size);
  return v->xs + index * sizeof_x;
}

void* _vector_emplace_back(vector_t* v, size_t sizeof_x) {
  if (v->size == v->capacity) {
    v->capacity *= 2;
    void* tmp = realloc(v->xs, v->capacity * sizeof_x);
    if (!tmp) {
      printf("vector: out of memory");
      exit(-1);
    }
    v->xs = tmp;
  }
  return v->xs + (v->size++) * sizeof_x;
}

void* _vector_start(vector_t* v, size_t sizeof_x) { return v->xs; }

void* _vector_end(vector_t* v, size_t sizeof_x) {
  return v->xs + v->size * sizeof_x;
}

int _vector_is_empty(vector_t* v, size_t sizeof_x) { return v->size == 0; }
