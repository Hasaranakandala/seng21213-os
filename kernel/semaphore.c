#include "semaphore.h"

void sem_init(semaphore_t* s, int count) {
    s->count = count;
}

void sem_wait(semaphore_t* s) {
    while (s->count <= 0) {
    }
    s->count--;
}

void sem_signal(semaphore_t* s) {
    s->count++;
}
