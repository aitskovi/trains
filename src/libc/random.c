/*
 * random.c
 *
 *  Created on: 2013-05-30
 *      Author: alex
 */

#include <random.h>

unsigned int rand_int(Random * random) {
    random->seed = (random->seed/1000) % 1000000;
    random->seed = random->seed * random->seed;
    return random->seed;
}

void seed_random(Random *random, unsigned int seed) {
    random->seed = seed;
}
