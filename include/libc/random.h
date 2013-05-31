/*
 * random.h
 *
 *  Created on: 2013-05-30
 *      Author: alex
 */

#ifndef RANDOM_H_
#define RANDOM_H_

typedef struct Random {
    unsigned int seed;
} Random;

unsigned int rand_int(Random * random);
void seed_random(Random *random, unsigned int seed);


#endif /* RANDOM_H_ */
