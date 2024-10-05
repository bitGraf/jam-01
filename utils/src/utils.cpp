#include "utils.h"

#include <stdlib.h>

static unsigned int seed = 123456;

void rand_seed() {
    srand(seed);
}
int16 rand_int(int16 min_val, int16 max_val) {
    return (int16)(rand() % (max_val + 1 - min_val) + min_val);
}