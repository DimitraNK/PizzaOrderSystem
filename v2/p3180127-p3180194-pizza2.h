// Including all the necessary header files.
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Setting constants equal to the given values.
const int N_cook = 2;
const int N_oven = 5;
const int N_deliverer = 10;
const int T_orderlow = 1;
const int T_orderhigh = 5;
const int N_orderlow = 1;
const int N_orderhigh = 5;
const int T_prep = 1;
const int T_bake = 10;
const int T_low = 5;
const int T_high = 15;