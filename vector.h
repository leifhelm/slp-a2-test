#ifndef VECTOR_H_
#define VECTOR_H_

#include <stddef.h>

#define DEFAULT_VECTOR_SIZE 16

typedef struct {
  void* xs;
  size_t capacity;
  size_t size;
} vector;

void _vector_init(vector* v, size_t sizeof_x);
void _vector_free(vector* v, size_t sizeof_x);

void* _vector_get(vector* v, size_t index, size_t sizeof_x);
void* _vector_emplace_back(vector* v, size_t sizeof_x);

void* _vector_start(vector* v, size_t sizeof_x);
void* _vector_end(vector* v, size_t sizeof_x);

#define vector_init(type, vector) _vector_init((vector), sizeof(type))
#define vector_free(type, vector) _vector_free((vector), sizeof(type))
#define vector_get(type, vector, index)                                        \
  ((type*) _vector_get((vector), (index), sizeof(type)))
#define vector_emplace_back(type, vector, index)                               \
  ((type*) _vector_emplace_back((vector), sizeof(type)))

#define vector_start(type, vector) ((type*) _vector_start(vector, sizeof(type)))
#define vector_end(type, vector) ((type*) _vector_end(vector, sizeof(type)))

#endif // VECTOR_H_
