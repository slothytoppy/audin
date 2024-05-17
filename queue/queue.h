#ifndef QUEUE_H
#define QUEUE_H
#include <assert.h>
#include <stdlib.h>

typedef struct {
  void** items;
  unsigned long count;
  unsigned long capacity;
  unsigned long cursor;
} Queue;

void queue_init(Queue*);
void queue_deinit(Queue*);
void queue_append(Queue*, void*);

#endif
