/*
   Copyright 2025 Mahdi Almusaad (https://github.com/mahdialmusaad)
   under the MIT License (https://opensource.org/license/mit)

   Pi calculations using infinite series - the sum of the terms of infinite
   sequences. The more sums/terms that are used with a given formula, the
   closer the result gets to the true value of pi.

   Infinite sums do so using addition whereas infinite products use multiplication.

   See https://en.wikipedia.org/wiki/Pi#Infinite_series for more information.
*/

/* Required includes. */
#include "c_threads.h" 
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

/* Types to use for storing calculations. */
typedef long terms_t; /* Loops counter type. */
typedef double pi_res_t; /* Calculation type. Preferably a floating point type. */

/* Shortcut for do..while loop with 'terms' counter. */
#define LOOP_TERMS(expr) do expr while(--terms)


/*
   Execution functions declarations.
*/

/* 
   Do a single pi calculation using the function correlation to the given index,
   printing out the result and the time taken to execute.
   Takes in the function index and the number of terms to calculate (aka accuracy).
*/
void do_series(int series_index, int given_terms_count);

/* 
   Same as do_series, but in a threading context.
   Takes in a pointer to a thread_series_data object, which is freed.
   Returns NULL for threading purposes.
*/
thread_func_t do_series_thread(thread_arg_t data);

/* Prints the given number with a comma for a thousands separator to stdout. */
void print_thousands_sepd_num(terms_t terms);

/*
   Infinite series functions declarations.
   All take in the number of terms (loop count).
*/

/* https://en.wikipedia.org/wiki/Wallis_product */
pi_res_t wallis_product(terms_t terms);

/* https://en.wikipedia.org/wiki/Vi%C3%A8te%27s_formula */
pi_res_t vietes_formula(terms_t terms);

/* https://en.wikipedia.org/wiki/Pi#cite_ref-FOOTNOTEArndtHaenel2006Formula_16.10,_p._223_78-0 */
pi_res_t nilakantha(terms_t terms);

/* 
   https://en.wikipedia.org/wiki/Arctangent_series
   https://en.wikipedia.org/wiki/Leibniz_formula_for_%CF%80
   Calculates 4 arctan(1). Newton's version converges much faster than this.
   The generalized formula of this for any arctan x is known as the Gregory series.
*/
pi_res_t madhava_leibniz_formula(terms_t terms);

/* 
   https://en.wikipedia.org/wiki/Pi#cite_ref-70
   Infinite series to calculate 4 arctan(1).
   Note that arctan 1 = pi/4.
*/
pi_res_t newton_arctan_pi(terms_t terms);


/*
   Struct definitions.
*/

/* Data struct for pi calculation in separate threads. */
typedef struct {
	int series_index;
	terms_t terms_count;
} thread_series_data;

/* Storage of functions and name for printing */
typedef struct {
	pi_res_t (*address)(terms_t terms);
	const char *given_name;
} series_func_data;

/* All calculation functions and names for display */
static series_func_data series_function_data[] = {
	{ wallis_product, "Wallis product" },
	{ vietes_formula, "Viete's formula" },
	{ nilakantha, "Nilakantha series" },
	{ madhava_leibniz_formula, "Madhava-Leibniz formula (arctan)" },
	{ newton_arctan_pi, "Newton series (arctan)" }
};


int main(int argc, char *argv[]) {
	/* Number of elements in series functions data list. */
	const int count_series_functions = (int)(sizeof series_function_data / sizeof *series_function_data);

	/* Check for correct argument count. */
	if (argc != 3) {
		fprintf(stderr, "Usage: %s series_choice series_terms\nChoices:\nall - All below series\n", *argv);
		for (int i = 0; i < count_series_functions; ++i) printf("  %d - %s\n", i + 1, series_function_data[i].given_name);
		return EXIT_FAILURE;
	}

	/* Determine if 'all' was chosen, otherwise the calculation function index. */
	const int is_all_series = *argv[1] == 'a';
	const pi_i64 given_series_value = is_all_series ? -1 : strtol(argv[1], NULL, 10);

	/* Check for a valid range of the possibly given function index. */
	if (!is_all_series && (given_series_value < 1 || given_series_value > count_series_functions)) {
		fprintf(stderr, "Invalid series option.\n");
		return EXIT_FAILURE;
	}

	/* Get the number of terms (times to loop) from the given 3rd argument. */
	const terms_t given_terms_count = strtol(argv[2], NULL, 10);
	
	/* Check for term value validity. */
	if (given_terms_count <= 0) {
		fprintf(stderr, "Terms value must be larger than 0.\n");
		return EXIT_FAILURE;
	}
	
	/* Print the number of terms formatted with commas. */
	printf("Terms: ");
	print_thousands_sepd_num(given_terms_count);
	printf("\nChosen series: %s\n", is_all_series ? "All" : series_function_data[given_series_value - 1].given_name);

	if (!is_all_series) {
		do_series(given_series_value - 1, given_terms_count);
		return EXIT_SUCCESS;
	}
	
	thread_id_t thread_handlers[count_series_functions];
	for (int i = 0; i < count_series_functions; ++i) {
		thread_series_data *const current_data = (thread_series_data*)malloc(sizeof(thread_series_data));
		if (!current_data) {
			fprintf(stderr, "Could not allocate memory for series \"%s\". Skipping.", series_function_data[i].given_name);
			continue;
		}
		
		current_data->series_index = i;
		current_data->terms_count = given_terms_count;
		pidef_create_thread(thread_handlers + i, do_series_thread, current_data);
	}

	for (int i = 0; i < count_series_functions; ++i) pidef_join_thread(thread_handlers[i]);
	return EXIT_SUCCESS;
}


/*
   Infinite series functions definitions.
*/

pi_res_t wallis_product(terms_t terms) {
	pi_res_t res = 1.0, top = 0.0, bottom = 1.0;
	LOOP_TERMS({
		top += 2.0;
		res *= (top / bottom);
		bottom += 2.0;
		res *= (top / bottom);
	});
	return res * 2.0;
}

pi_res_t vietes_formula(terms_t terms) {
	pi_res_t res = 1.0, sqr_res = 0.0;
	LOOP_TERMS( res *= 2.0 / (sqr_res = sqrt(2.0 + sqr_res)); );
	return res * 2.0;
}

pi_res_t nilakantha(terms_t terms) {
	pi_res_t res = 3.0, denom_cnt = 2.0, sign = -1.0, denom = 0.0;
	LOOP_TERMS({
		denom = denom_cnt * (denom_cnt + 1.0);
		res += (4.0 / (denom *= (denom_cnt += 2.0))) * (sign = -sign); 
	});
	return res;
}

pi_res_t madhava_leibniz_formula(terms_t terms) {
	pi_res_t res = 1.0, sign = 1.0, denom = 1.0;
	LOOP_TERMS( res += (1.0 / (denom += 2.0)) * (sign = -sign); );
	return res * 4.0;
}

pi_res_t newton_arctan_pi(terms_t terms) {
	pi_res_t res = 0.5, fract_num = 0.0, fract_den = 1.0, fract_tot = 1.0, den_mult = 2.0;
	LOOP_TERMS( res += (1.0 / (den_mult *= 2.0)) * (fract_tot *= ((fract_num += 2.0) / (fract_den += 2.0))); );
	return res * 4.0;
}


/*
   Execution functions definitions.
*/

void do_series(int series_index, int given_terms_count) {
	/* Get the specific calculation function from the given index. */
	const series_func_data current_series = series_function_data[series_index];

	/* Time calculating pi with specific function. */
	const clock_t start_time = clock();
	const pi_res_t res = current_series.address(given_terms_count);
	const clock_t end_time = clock();

	/* Print result of specific function and the time it took in seconds. */
	printf("%s result: %f (%fs)\n", current_series.given_name, res, (double)(end_time - start_time) / CLOCKS_PER_SEC);
}

thread_func_t do_series_thread(thread_arg_t data) {
	thread_series_data given_data = *(thread_series_data*)(data); /* Cast data from main thread to proper type. */
	free(data); /* Given pointer is created with malloc. */
	do_series(given_data.series_index, given_data.terms_count); /* Calculate and print results. */
	return NULL; /* Threading functions return a void pointer. Not used in this case, so it can be left as NULL. */
}

void print_thousands_sepd_num(terms_t terms) {
	if (terms >= 1000) {
		print_thousands_sepd_num(terms / 1000); /* Recursion to advance through number. */
		printf(",%03ld", terms % 1000); /* Print next 3 digits */
	} else printf("%ld", terms); /* Print whole number if it is less than 1k, no separator needed*/
}
