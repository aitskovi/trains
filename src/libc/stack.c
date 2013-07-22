/*
 * stack.c
 *
 *  Created on: Jul 22, 2013
 *      Author: aianus
 */


#include <stack.h>


unsigned int stack_size(stack *s) {
    return s->pos;
}
void stack_push(stack *s, void *x) {
    if (stack_size(s) >= MAX_STACK_SIZE) {
        ulog("Overflowed stack!!!");
        return;
    }
    s->elements[s->pos++] = x;
}
void *stack_pop(stack *s) {
    if (stack_size(s) == 0) return 0;
    return s->elements[--(s->pos)];
}

void *stack_initialize(stack *s) {
    s->pos = 0;
}
