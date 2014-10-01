#ifndef _ITERATOR_H
#define _ITERATOR_H

typedef struct iterator_t iterator;

struct iterator_t {
    const void *data;
    void *vnow;
    int inow;
    int ended;
    void* (*next)(iterator*);
    void* (*prev)(iterator*);
};

void* iter_next(iterator *iterator);
void* iter_prev(iterator *iterator);

#endif // _ITERATOR_H
