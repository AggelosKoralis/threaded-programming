#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdalign.h>

typedef struct {
    alignas(64) long long int n;
} counter_t;

struct array_stats_s {
    counter_t info_array_0;
    counter_t info_array_1;
    counter_t info_array_2;
    counter_t info_array_3;
} __attribute__ ((aligned(64))) array_stats;

// global stat variables
struct array_stats_s thread_stats = {
    .info_array_0.n = 0,
    .info_array_1.n = 0,
    .info_array_2.n = 0,
    .info_array_3.n = 0
};
struct array_stats_s serial_stats = {
    .info_array_0.n = 0,
    .info_array_1.n = 0,
    .info_array_2.n = 0,
    .info_array_3.n = 0
};

typedef struct {
    int *array;
    long long size; 
    int id;
} thread_args;

void *worker(void *args) {
    thread_args *a = (thread_args *) args;
    int *array = a->array;
    long long num = a->size;
    int id = a->id;

    for (int i = 0; i < num; i++) {
        if (array[i] != 0) 
            continue;
    
        switch (id) {
        case 0:
            thread_stats.info_array_0.n++;
            break;
        case 1:
            thread_stats.info_array_1.n++;
            break;
        case 2:
            thread_stats.info_array_2.n++;
            break;
        case 3:
            thread_stats.info_array_3.n++;
            break;
        default:
            break;
        }
    }

    return NULL;
}

// counts elements that equal 0
// returns the time it took to complete
float threaded(int *a[4], long long num) {
    pthread_t threads[4];
    thread_args args[4];
    struct timespec start, end;

    for (int i = 0; i < 4; i++) {
        args[i].array = a[i];
        args[i].size = num;
        args[i].id = i;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < 4; i++)
        pthread_create(&threads[i], NULL, worker, (void *)&args[i]);

    for (int i = 0; i < 4; i++)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);

    return (float) (end.tv_sec - start.tv_sec) + (float) (end.tv_nsec - start.tv_nsec) / 1e9;
}

// counts elements that equal 0
// returns the time it took to complete
float serial(int *a[4], long long num) {
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < num; j++) {
            if (a[i][j] != 0) 
                continue;

            switch (i) {
            case 0:
                serial_stats.info_array_0.n++;
                break;
            case 1:
                serial_stats.info_array_1.n++;
                break;
            case 2:
                serial_stats.info_array_2.n++;
                break;
            case 3:
                serial_stats.info_array_3.n++;
                break;
            default:
                break;
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    return (float) (end.tv_sec - start.tv_sec) + (float) (end.tv_nsec - start.tv_nsec) / 1e9;
}


void fill_arrays(int *a[4], long long num) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < num; j++)
            a[i][j] = rand() % 5;
}

int alloc_arrays(int *a[4], long long num) {
    for (int i = 0; i < 4; i++) {
        a[i] = malloc(sizeof(int) * num);
        
        if (!a[i]) {
            for (int j = 0; j < i; j++)
                free(a[j]);

            return 1;
        }
    }

    return 0;
}

int prepare_arrays(int *a[4], long long num, float *array_time) {
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    
    if (alloc_arrays(a, num) == 1) 
        return 1;

    fill_arrays(a, num);

    clock_gettime(CLOCK_MONOTONIC, &end);
    *array_time = (float) (end.tv_sec - start.tv_sec) + (float) (end.tv_nsec - start.tv_nsec) / 1e9;
    
    return 0;
}

void print_stats(struct array_stats_s stats) {
    printf("0s in array %d: %lld\n", 0, stats.info_array_0.n);
    printf("0s in array %d: %lld\n", 1, stats.info_array_1.n);
    printf("0s in array %d: %lld\n", 2, stats.info_array_2.n);
    printf("0s in array %d: %lld\n", 3, stats.info_array_3.n);
}

int same_result(void) {
    return (thread_stats.info_array_0.n == serial_stats.info_array_0.n) &&
           (thread_stats.info_array_1.n == serial_stats.info_array_1.n) &&
           (thread_stats.info_array_2.n == serial_stats.info_array_2.n) &&
           (thread_stats.info_array_3.n == serial_stats.info_array_3.n);
}

void print_improvement(float serial_time, float thread_time) {
    float improvement = 100 * (serial_time - thread_time) / serial_time;

    if (improvement < 0.0)
        printf("The parallel algorithm is %f%% slower\n", -1 * improvement);
    else 
        printf("The parallel algorithm is %f%% faster\n", improvement);
    
}

int run_serial(int *a[4], long long num, float *serial_time) {
    *serial_time = serial(a, num);

    FILE *serialfile = fopen("serial.txt", "a");
    if (!serialfile) 
        return 1;

    fprintf(serialfile, "%lld %f\n", num, *serial_time);
    fflush(serialfile);
    fclose(serialfile);

    return 0;
}

int run_threaded(int *a[4], long long num, float *thread_time) {
    *thread_time = threaded(a, num);

    FILE *threadedfile = fopen("threaded.txt", "a");
    if (!threadedfile) 
        return 1;

    fprintf(threadedfile, "%lld %f\n", num, *thread_time);
    fflush(threadedfile);
    fclose(threadedfile);

    return 0;
}

int main(int argc, char **argv) {
    
    if (argc != 2) {
        printf("main: Usage: ./ask3 <number of elements of each array>.\n");
        exit(1);
    }
    
    srand(time(NULL));
    long long num = atoi(argv[1]);
    float array_time, serial_time, thread_time;

    if (num <= 0) {
        printf("main: Input must be a positive integer.\n");
        exit(1);
    }

    int *a[4];
    if (prepare_arrays(a, num, &array_time) == 1) {
        printf("Error while preparing the arrays.\n");
        exit(1);
    }

    if (run_serial(a, num, &serial_time) == 1) {
        printf("Error while runnig the serial algorithm.\n");
        
        for (int i = 0; i < 4; i++) 
            free(a[i]);

        exit(1);
    }

    if (run_threaded(a, num, &thread_time) == 1) {
        printf("Error while runnig the threaded algorithm.\n");

        for (int i = 0; i < 4; i++) 
            free(a[i]);
        
        exit(1);
    }

    if (same_result())
        printf("Both methods got the same results\n");
    else 
        printf("Error: different results\n");

    print_improvement(serial_time, thread_time);

    for (int i = 0; i < 4; i++) 
        free(a[i]); 

    return 0;
}
