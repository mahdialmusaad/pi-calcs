/* 
   Copyright 2025 Mahdi Almusaad (https://github.com/mahdialmusaad)
   under the MIT License (https://opensource.org/license/mit)

   Using the Monte Carlo method, we can approximate pi by generating random 'points'
   on a 'unit square' (coordinates lie in range [0-1]) and using the ratio of the number
   of 'points' inside a 'quadrant' (i.e. a quarter-circle) on the square to the total 'points'
   generated, resulting in a value that converges on pi/4 as the number of 'points' approaches infinity.

   This specific Monte Carlo method is easily parallelized and easy to calculate as
   it only involves generating random points and calculating their length and an overall ratio,
   with no dependency on previous iterations.

   However, it is hopelessly inaccurate, only allowing calculations of a very few digits of pi even
   as the number of iterations is heavily increased.
   
   See https://en.wikipedia.org/wiki/Monte_Carlo_method and
   https://en.wikipedia.org/wiki/Pi#Monte_Carlo_methods for more information.
   
   This is the C version of this example.
   The CUDA version is available within the same repository (pi_mcarlo_cuda.cu).
*/

/* Required includes. */
#include "c_threads.h"
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

typedef uint_least64_t counter_t; /* Int type to store the number of 'points' during calculation. */
static counter_t num_iterations; /* Run-time value of the number of points. */

/*
   Function declarations.
*/

/* Returns a 'pseudo-random' decimal in the range [0, 1] using the provided seed.
   Modifies the seed for the next call. */
float fastrand01(unsigned *seed);

/* 
   Generates 'num_iterations' random points on a unit square to determine how
   many are within a quadrant for use in a Monte Carlo pi approximation.

   Takes in a void* (pointer) to two 'counter_t' integers.

   After completion, the first integer stores the number of 'points' that lie
   'outside' the 'unit square' and the second stores the number of 'inside points'.

   The ratio of the number of 'inside points' and *total* 'points' converges on pi/4 as 'num_iterations' approaches infinity.
*/
thread_func_t approximate_pi_mcarlo(thread_arg_t results_array);

int main(int argc, char *argv[]) {
	/* Check for the correct number of arguments. */
	if (argc != 3) {
		fprintf(stderr, "Usage: %s points_per_thread num_threads\n", *argv);
		return EXIT_FAILURE;
	}

	num_iterations = strtoul(argv[1], NULL, 10);
	const long num_threads = strtol(argv[2], NULL, 10);

	counter_t thread_results[num_threads][2]; /* Create empty 'results' arrays for each thread to work on. */
	memset(thread_results, 0, sizeof(thread_results));
	
	/* Begin timing. */
	const clock_t start_time = clock();

	/* Create all of the other threads to work on their own section of the result arrays. */
	#if num_threads > 1
	thread_id_t threads[num_threads - 1];
	for (int i = 0; i < num_threads - 1; ++i) pidef_create_thread(&threads[i], approximate_pi_mcarlo, thread_results[i]);
	#endif
	
	/* Make the main thread also calculate instead of slouching around. */
	counter_t *local_results = thread_results[num_threads - 1]; /* Using last results counters. */
	approximate_pi_mcarlo(local_results);

	/* Wait for all other threads to finish. */
	#if num_threads > 1
	for (int i = 0; i < num_threads - 1; ++i) pidef_join_thread(threads[i]);

	/* Combine all of the results into the main thread's counters. */
	for (int i = 0; i < num_threads - 1; ++i) {
		local_results[0] += thread_results[i][0];
		local_results[1] += thread_results[i][1];
	}
	#endif

	/* End timing. */
	const clock_t end_time = clock();

	/* Print overall counters results and pi from points ratio. */
	printf("Points results:\n  %lu inside\n  %lu outside\nPi approximation: %f\nTime taken: %fs\n",
		local_results[1], local_results[0],
		(4.0 * (double)local_results[1]) / (double)(local_results[0] + local_results[1]),
		(double)(end_time - start_time) / CLOCKS_PER_SEC
	);

	return 0;
}

/*
   Function definitions.
*/

float fastrand01(unsigned *seed) { return (float)( (*seed = 3812762923u * (*seed)) & RAND_MAX) / (float)RAND_MAX; }

thread_func_t approximate_pi_mcarlo(thread_arg_t results_array) {
	/* Cast given void pointer into a counter_t integer pointer type. */
	counter_t *counter_arrays = (counter_t*)results_array;
	
	/* Create a local seed value using normal 'rand' and time functions. */
	unsigned seed = (rand() + 214584u) * time(NULL);

	for (counter_t i = 0; i < num_iterations; ++i) {
		/* Get a pseudo-random X and Y position, each in the range [0, 1]. */
		const float randX = fastrand01(&seed), randY = fastrand01(&seed);
		/* The point is 'inside' the 'circle' if the coordinate sqr. length (X^2 + Y^2) is less than 1.
		   'Use conditional result (<1.0 = 1 else 0) for indexes into pointer. */
		++counter_arrays[(randX * randX) + (randY * randY) < 1.0];
	}

	return NULL;
}
