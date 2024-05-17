#include "./queue.h"
#include <stdio.h>

void queue_init(Queue* q) {
  q->count = 0;
  q->capacity = 8;
  q->cursor = 0;
  q->items = calloc(8, sizeof(void*));
}

void queue_deinit(Queue* q) {
  q->count = 0;
  q->capacity = 0;
  for(int i = 0; i < q->count; i++) {
    free(q->items[i]);
  }
  free(q->items);
}

void queue_append(Queue* q, void* data) {
  if(q->count == 0 || q->capacity == 0) {
    q->items = calloc(8, sizeof(void*));
    q->capacity = 8;
  }
  if(q->count + 1 > q->capacity) {
    q->capacity *= 2;
    q->items = realloc(q->items, sizeof(q->items) * (q->capacity + 1));
  }
  assert(q->items != NULL);
  q->count += 1;
  q->items[q->count - 1] = data;
}
