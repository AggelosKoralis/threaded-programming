#ifndef ASK1_HELPER_H
#define ASK1_HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define COEF_CAP 100

typedef long long degree_t;
typedef long long coef_t;

typedef struct {
    degree_t degree;
    coef_t *coef;
    float init_time;
} polynomial;

typedef struct {
    int id;
    int thread_count;
    polynomial *pol1;
    polynomial *pol2;
    polynomial *pol;
} thread_args;

#endif
