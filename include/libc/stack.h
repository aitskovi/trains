/*
 * stack.h
 *
 *  Created on: Jul 22, 2013
 *      Author: aianus
 */

#ifndef STACK_H_
#define STACK_H_

#define MAX_STACK_SIZE 100

typedef struct {
    void *elements[MAX_STACK_SIZE];
    unsigned int pos;
} stack;

unsigned int stack_size(stack *s);
void stack_push(stack *s, void *x);
void *stack_pop(stack *s);
void *stack_initialize(stack *s);

#endif /* STACK_H_ */
