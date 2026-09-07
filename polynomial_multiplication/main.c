#include "helper.h"
#include <time.h>

typedef enum {
OK              = 0,
ARG_NUM         /* 1 */,
ARG_VAL         /* 2 */,
ALLOC_POL1      /* 3 */,
ALLOC_POL2      /* 4 */,
POL_SERIAL      /* 5 */,
WRITE_SERIAL    /* 6 */,
POL_PARALLEL    /* 7 */,
WRITE_PARALLEL  /* 8 */,
BAD_RES         /* 9 */
} RETURN_CODES;


void *thread_func(void* args) {
    thread_args myargs = *(thread_args *) args;

    int id = myargs.id;
    int thread_count = myargs.thread_count;
    polynomial *pol1 = myargs.pol1;
    polynomial *pol2 = myargs.pol2;
    polynomial *res  = myargs.pol;

    degree_t deg1 = pol1->degree;
    degree_t deg2 = pol2->degree;
    degree_t result_deg = deg1 + deg2;

    degree_t chunk = (result_deg + 1 + thread_count - 1) / thread_count;
    degree_t start = id * chunk;
    degree_t end   = start + chunk;
    if (end > result_deg + 1) end = result_deg + 1;

    for (degree_t k = start; k < end; k++) {
        long long sum = 0;

        degree_t i_start = (k - deg2 > 0) ? (k - deg2) : 0;
        degree_t i_end   = (k < deg1)    ? k          : deg1;

        for (degree_t i = i_start; i <= i_end; i++) {
            degree_t j = k - i;
            sum += pol1->coef[i] * pol2->coef[j];
        }

        res->coef[k] = sum;
    }

    return NULL;
}


// 1 -> equal
// 0 -> not equal
int equal(polynomial *pol1, polynomial *pol2) {
    if (pol1->degree != pol2->degree)
        return 0;

    for (degree_t i = 0; i <= pol1->degree; i++)
        if (pol1->coef[i] != pol2->coef[i])
            return 0;

    return 1;
}


void polynomial_generator(polynomial *pol) {
    coef_t coef;
    
    if(!pol) return;

    for (degree_t i = 0; i <= pol->degree; i++) {
        coef = rand() % COEF_CAP;
        // get some negative numbers
        if (rand() % 2) 
            coef -= COEF_CAP / 2;

        // somewhat avoid coefficients that equal 0
        if (coef == 0)
            coef += rand() % COEF_CAP + COEF_CAP / 2;

        pol->coef[i] = coef;
    }
}


// allocates mem for a polynomial
polynomial *new_pol(degree_t degree) {
    polynomial *pol = malloc(sizeof(polynomial));
    if (!pol) return NULL;
    
    pol->coef = calloc(degree + 1, sizeof(coef_t));
    if (!pol->coef){
        free(pol);
        return NULL;
    }
    pol->degree = degree;
    pol->init_time = 0.0;

    return pol;
}


// serial polyonymial multiplication
polynomial *serial_multiplication(polynomial *pol1, polynomial *pol2) {
    struct timespec start, end;
    float diff_time;
    degree_t result_degree;
    polynomial *result;

    clock_gettime(CLOCK_REALTIME, &start);

    result_degree = pol1->degree + pol2->degree;
    result = new_pol(result_degree);
    if (!result) return NULL;

    for (degree_t i = 0; i <= pol1->degree; i++)
        for (degree_t j = 0; j <= pol2->degree; j++)
            result->coef[i + j] += (pol1->coef[i] * pol2->coef[j]);

    clock_gettime(CLOCK_REALTIME, &end);

    diff_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    result->init_time = diff_time;

    return result;
}


polynomial *parallel_multiplication(polynomial *pol1, polynomial *pol2, int thread_count) {
    struct timespec start, end;
    float diff_time;
    pthread_t *threads;
    thread_args *args;
    polynomial *parallel_mult;

    threads = malloc(thread_count * sizeof(pthread_t));
    if (!threads) {
        printf("parallel_multiplication: Memory allocation for the threads failed.\n");
        return NULL;
    }

    args = malloc(thread_count * sizeof(thread_args));
    if (!args) {
        printf("parallel_multiplication: Memory allocation for the threads args failed.\n");
        free(threads);
        return NULL;
    }

    parallel_mult = new_pol(pol1->degree + pol2->degree);
    if (!parallel_mult) {
        printf("parallel_multiplication: Memory allocation for the parallel polynomial failed.\n");
        free(threads);
        free(args);
        return NULL;
    }

    for (int t = 0; t < thread_count; t++) {
        args[t].id = t;
        args[t].thread_count = thread_count;
        args[t].pol1 = pol1;
        args[t].pol2 = pol2;
        args[t].pol = parallel_mult;
    }

    clock_gettime(CLOCK_REALTIME, &start);
    for (int t = 0; t < thread_count; t++)
        pthread_create(&threads[t], NULL, thread_func, (void *) &args[t]); // omg omgggg

    for (int i = 0; i < thread_count; i++) 
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_REALTIME, &end);

    diff_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    parallel_mult->init_time = diff_time;

    free(args);
    free(threads);
    return parallel_mult;    
}


polynomial *pol_init(degree_t degree, int id) {
    struct timespec start, end;
    float diff_time;
    polynomial *pol;

    clock_gettime(CLOCK_REALTIME, &start);
    pol = new_pol(degree);
    polynomial_generator(pol);
    clock_gettime(CLOCK_REALTIME, &end);

    if (!pol) {
        printf("pol_init: Memory allocation  for polynom %d failed.\n", id);
        return NULL;
    }

    diff_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    pol->init_time = diff_time;

    return pol;
}


// log some stats for the result polynomials
int save_to_file(const char *fname, polynomial *pol) {
    FILE *outfile = fopen(fname, "a");
    if (!outfile) {
        printf("pol_init: failed to open <%s>.\n", fname);
        return 1;
    }

    fprintf(outfile, "%lld %f\n", pol->degree, pol->init_time);
    fflush(outfile);
    fclose(outfile);

    return 0;
}


int main(int argc, char **argv) {
    RETURN_CODES return_code = OK;
    degree_t degree1, degree2;
    int thread_count;
    polynomial *pol1, *pol2, *serial_mult, *parallel_mult;
    char threaded_filename[32];
    
    if (argc != 4) {
        printf("main: Usage: ./ask1 <degree pol1> <degree pol2> <threads>.\n");
        return_code = ARG_NUM;
        goto out;
    }
    
    srand(time(NULL));

    degree1 = atoll(argv[1]);
    degree2 = atoll(argv[2]);
    thread_count = atoi(argv[3]);

    if (degree1 <= 0 || degree2 <= 0 || thread_count <= 0) {
        printf("main: All arguements must be positive integers.\n");
        return_code = ARG_VAL;
        goto out;
    }

    pol1 = pol_init(degree1, 1);
    if (!pol1) {
        return_code = ALLOC_POL1;
        goto out;
    }

    pol2 = pol_init(degree2, 2);
    if (!pol2) {
        return_code = ALLOC_POL2;
        goto free_pol1;
    }


    serial_mult = serial_multiplication(pol1, pol2);
    if (!serial_mult) {
        printf("main: Memory allocation at serial multiplication failed.\n");
        return_code = POL_SERIAL;
        goto free_pol2;
    }
    
    if (save_to_file("results/serial.txt", serial_mult) == 1) {
        return_code = WRITE_SERIAL;
        goto free_serial;
    }


    parallel_mult = parallel_multiplication(pol1, pol2, thread_count);
    if (!parallel_mult) {
        return_code = POL_PARALLEL;
        goto free_serial;
    }
   
    snprintf(threaded_filename, 32, "results/threaded_%d.txt", thread_count);
    if (save_to_file(threaded_filename, parallel_mult) == 1) {
        return_code = WRITE_PARALLEL;
        goto free_parallel;
    }
    

    if (!equal(serial_mult, parallel_mult)) {
        printf("main: The resulting polynomials are not the same.\n");
        return_code = BAD_RES;
    }

free_parallel:
    free(parallel_mult->coef);
    free(parallel_mult);
free_serial:
    free(serial_mult->coef);
    free(serial_mult);
free_pol2:
    free(pol2->coef);
    free(pol2);
free_pol1:
    free(pol1->coef);
    free(pol1);
out:
    return return_code;
}
