#include <pthread.h>
#include <stdio.h>

int i = 0;
pthread_t thread[10];
pthread_attr_t attr;
pthread_mutex_t mutex;
pthread_mutexattr_t mutexattr;

typedef struct state {
  int i;
  char* msg;
} State;

State state = {0};

void* printi(void* a) {
  (void)a;
  pthread_mutex_lock(&mutex);
  state.i += 1;
  printf("%d\n", state.i);
  printf("%s\n", state.msg);
  pthread_mutex_unlock(&mutex);
  return a;
}

int main(void) {
  state.msg = "yes";
  pthread_mutexattr_init(&mutexattr);
  pthread_mutex_init(&mutex, &mutexattr);
  pthread_attr_init(&attr);
  void* state = (State*)state;
  for(int i = 0; i < 10; i++) {
    pthread_create(&thread[i], &attr, printi, state);
    pthread_join(thread[i], NULL);
  }
  return 0;
}
