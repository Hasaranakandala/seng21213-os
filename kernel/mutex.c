#include "mutex.h"

void mutex_init(mutex_t* m) {
    m->locked = 0;
}

void mutex_lock(mutex_t* m) {
    while (__atomic_test_and_set(&(m->locked), __ATOMIC_ACQUIRE)) {
        // Spinlock - බලාගෙන සිටීම
    }
}

void mutex_unlock(mutex_t* m) {
    __atomic_clear(&(m->locked), __ATOMIC_RELEASE);
}
