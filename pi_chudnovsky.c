/* 
   Copyright 2025 Mahdi Almusaad (https://github.com/mahdialmusaad)
   under the MIT License (https://opensource.org/license/mit)

   The Chudnovsky algorithm calculates pi with a large amount of accuracy, and is used
   for all recent pi digit calculation world records. Internally, it uses a modified version on
   Ramanujan's infinite series of pi.

   This uses an optimization technique called 'binary splitting' to speed up the calculation
   instead of relying on the raw formula. The result is an *integer* value that represents the value
   of pi (i.e. 3141... instead of 3.141...).

   The original Python source can be seen here: https://www.craig-wood.com/nick/articles/pi-chudnovsky/
   This is a C adaptation of the Python source.

   See the following articles for more information:
   https://en.wikipedia.org/wiki/Ramanujan%E2%80%93Sato_series
   https://en.wikipedia.org/wiki/Chudnovsky_algorithm
   https://en.wikipedia.org/wiki/Binary_splitting
*/

/* Required includes. */
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Types to use for storing calculations. */
typedef uint_least64_t pi_uint;
typedef double pi_flt;

/* Structure used for calculations and results. */
typedef struct { pi_flt Pab, Qab, Tab; } result_flts;

/*
   Uses a binary-splitting version of Chudnovsky's algorithm to calculate pi.
   Returns a struct of specific values used to calculate an integer representation of pi.
*/
result_flts chudnovsky_binarysplit(pi_uint a, pi_uint b);

int main(int argc, char *argv[]) {
	/* Validate arguments count. */
	if (argc != 2) {
		fprintf(stderr, "Usage: %s pi_digits\n", *argv);
		return EXIT_FAILURE;
	}

	/* Get and validate pi digits accuracy. */
	const long digits = strtol(argv[1], NULL, 10) * 2;
	if (digits < 0)  {
		fprintf(stderr, "Digits count must be larger than 0.\n");
		return EXIT_FAILURE;
	} 

	/* Calculate pi and output the result. */
	const result_flts res = chudnovsky_binarysplit(0, (pi_uint)((pi_flt)digits / 14.181647462725477) + (pi_uint)(1U));
	const pi_uint pi = ((res.Qab * 426880.0 * sqrt(10005.0 * pow(10.0, (double)digits))) / res.Tab);
	printf("Pi approximation: %" PRIu64 "\n", pi);

	return EXIT_SUCCESS;
}

result_flts chudnovsky_binarysplit(pi_uint a, pi_uint b) {
	result_flts res;
	if (b - a == 1) {
		if (!a) res.Pab = res.Qab = 1.0;
		else {
			const pi_flt QabaM = 10939058860032000.0;
			res.Pab = (6.0 * a - 5.0) * (2.0 * a - 1.0) * (6.0 * a - 1.0);
			res.Qab = a * a * a * QabaM;
		}

		res.Tab = res.Pab * ((545140134.0 * a) + 13591409.0);
		res.Tab = a & 1 ? -res.Tab : res.Tab;
		return res;
	} else {
		const pi_uint m = (a + b) / (pi_uint)(2U);
		const result_flts am = chudnovsky_binarysplit(a, m);
		const result_flts mb = chudnovsky_binarysplit(m, b);

		res.Pab = am.Pab * mb.Pab;
		res.Qab = am.Qab * mb.Qab;
		res.Tab = mb.Qab * am.Tab + am.Pab * mb.Tab;
	}
	
	return res;
}
