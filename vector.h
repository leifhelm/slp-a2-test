#ifndef VECTOR_H_
#define VECTOR_H_

#include <stddef.h>

#define DEFAULT_VECTOR_CAPACITY 16

typedef struct {
  void* xs;
  size_t capacity;
  size_t size;
} vector_t;

void _vector_init_with_capacity(vector_t* v, size_t capacity, size_t sizeof_x);
void _vector_free(vector_t* v, size_t sizeof_x);

void* _vector_get(vector_t* v, size_t index, size_t sizeof_x);
void* _vector_emplace_back(vector_t* v, size_t sizeof_x);

void* _vector_start(vector_t* v, size_t sizeof_x);
void* _vector_end(vector_t* v, size_t sizeof_x);

int _vector_is_empty(vector_t* v, size_t sizeof_x);

#define vector_init(type, vector)                                              \
  _vector_init_with_capacity((vector), DEFAULT_VECTOR_CAPACITY, sizeof(type))
#define vector_init_with_capacity(type, vector, capacity)                      \
  _vector_init_with_capacity((vector), (capacity), sizeof(type))
#define vector_free(type, vector) _vector_free((vector), sizeof(type))
#define vector_get(type, vector, index)                                        \
  ((type*) _vector_get((vector), (index), sizeof(type)))
#define vector_emplace_back(type, vector)                                      \
  ((type*) _vector_emplace_back((vector), sizeof(type)))

#define vector_start(type, vector)                                             \
  ((type*) _vector_start((vector), sizeof(type)))
#define vector_end(type, vector) ((type*) _vector_end((vector), sizeof(type)))

#define vector_is_empty(type, vector) _vector_is_empty((vector), sizeof(type))

#endif // VECTOR_H_
