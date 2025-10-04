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

   See https://en.wikipedia.org/wiki/Monte_Carlo_method and https://en.wikipedia.org/wiki/Pi#Monte_Carlo_methods for more information.

   This is the CUDA version of this program.
   The C version is available within the same repository (pi_mcarlo.c).
*/

/* Required includes. */
#include <string.h>
#include <stdint.h>
#include <stdio.h>


/*
   Function declarations.
*/


/* Error checking macro and function. */
#define checkError(result) { checkCudaError(result, __LINE__); }
__host__ void checkCudaError(cudaError_t result, int line);

/* 
   Returns a 'pseudo-random' decimal in the range [0, 1] using the provided seed.
   Modifies the seed for the next call.

   This is marked with __device__ as it is only required for the below kernel function.
*/
__device__ float fastrand01(unsigned *seed);

/* 
   Generates a random 'point' for each thread on a unit square to determine
   if it is within a 'quadrant' for use in a Monte Carlo pi approximation.

   Takes in a pointer to some amount of bytes, depending on how many threads were given.

   After completion by a thread, either the first or second bit of each byte will be set. The first
   bit is changed if the point was outside otherwise the second if it was inside the 'quadrant'.

   The ratio of the number of 'inside points' and *total* 'points' converges on pi/4 as the total
   number of threads/number of stored results approaches infinity.

   This is marked with __global__ as it is a 'kernel' function (i.e. GPU thread code).
*/
__global__ void approximate_pi_mcarlo(uint8_t *results_array, time_t curr_time);

int main(int argc, char *argv[]) {
	/* Validate number of arguments. */
	if (argc != 3) {
		fprintf(stderr, "Usage: %s thread_blocks threads_per_block\n", *argv);
		return EXIT_FAILURE;
	}

	/* Get values from input. */
	const long num_thread_blocks = strtol(argv[1], NULL, 10);
	const long threads_per_block = strtol(argv[2], NULL, 10);
	const unsigned long total_thread_count = num_thread_blocks * threads_per_block;

	/* Validate given values. */
	if (num_thread_blocks <= 0 || threads_per_block <= 0) {
		fprintf(stderr, "Values must be positive.\n");
		return EXIT_FAILURE;
	}
	if (threads_per_block > 1024) {
		fprintf(stderr, "Too many threads per block.\n");
		return EXIT_FAILURE;
	}
	if (num_thread_blocks > INT32_MAX) {
		fprintf(stderr, "Too many thread blocks.\n");
		return EXIT_FAILURE;
	}

	printf("Using %lu threads\n", total_thread_count);

	/* Create an empty results array */
	uint_least64_t total_results[2];
	memset(&total_results, 0, sizeof total_results);
	
	/* Malloc a byte for each thread (using individual bits to store results) */
	uint8_t *thread_counters;
	const size_t results_size_bytes = sizeof *thread_counters * total_thread_count;
	checkError(cudaMallocManaged(&thread_counters, results_size_bytes));

	/* Prefetch allocated memory to avoid page faults */
	checkError(cudaMemPrefetchAsync(thread_counters, results_size_bytes, 0));

	/* Start calculating pi using explained method across a set amount of threads and blocks */
	approximate_pi_mcarlo<<<num_thread_blocks, threads_per_block>>>(thread_counters, time(NULL));

	/* Wait for calculations to complete (kernel code is non-blocking) */
	checkError(cudaDeviceSynchronize());

	/* Combine all kernel results into the host array */
	for (size_t i = 0; i < total_thread_count; ++i) ++total_results[thread_counters[i] - 1];

	/* Free malloc'd counters */
	checkError(cudaFree(thread_counters));

	/* Print overall counters results and pi from points ratio. */
	printf("Points results:\n  %lu inside\n  %lu outside\nPi approximation: %f\n",
		total_results[1], total_results[0],
		(4.0 * (double)total_results[1]) / (double)(total_results[0] + total_results[1])
	);

	return EXIT_SUCCESS;
}


/*
   Function definitions.
*/


__host__ void checkCudaError(cudaError_t result, int line)
{
	if (result != cudaSuccess) {
		fprintf(stderr, "CUDA error: %s\n", cudaGetErrorString(result));
		exit(EXIT_FAILURE);
	}
}

__device__ float fastrand01(unsigned *seed)
{
	return (float)( (*seed = 3812762923u * (*seed)) & RAND_MAX) / (float)RAND_MAX;
}

__global__ void approximate_pi_mcarlo(uint8_t *results_array, time_t curr_time)
{
	/* Calculate this thread's global index. */
	const unsigned threadIndex = (blockIdx.x * blockDim.x) + threadIdx.x;
	/* Create a local seed value using the calculated index and given time for the 'random' function.
	   It is also possible to use cuRand (https://developer.nvidia.com/curand) here. */
	unsigned seed = ((~threadIndex * 23687u) ^ 965724u) * curr_time; 
	/* Get a random X and Y position, each in the range [0, 1]. */
	const float randX = fastrand01(&seed), randY = fastrand01(&seed);
	/* The point is 'inside' the 'circle' if the coordinate's square length (X^2 + Y^2) is less than 1.0.
	   Using the condition's result (0 or 1) to determine the bit to set. */
	results_array[threadIndex] |= 1 + (randX * randX + randY * randY < 1.0);
}
