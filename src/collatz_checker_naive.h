#ifndef COLLATZ_CHECKER_NAIVE_H_
#define COLLATZ_CHECKER_NAIVE_H_

#include <gmpxx.h>
#include <stddef.h>

#include "collatz_multistep.h"
#include "mpz_utils.h"

class collatz_checker_naive {
public:
	mpz_class value = 1;

	size_t step_count_evn = 0;
	size_t step_count_odd = 0;
	size_t iter_count = 0;

	mpz_class& start_value_ref() {
		return value;
	}

	void start_value_modified() {
	}

	std::string type_abbrev() {
		return "naive";
	}

	size_t step_count() {
		return step_count_evn + step_count_odd;
	}

	void complete_check() {
		while (not_finished()) {
			iterate();
		}
	}

	void iterate() {
		if (value.get_ui() & 1) {
			value *= 3;
			value++;

			step_count_odd++;
		} else {
			size_t lowest_1_bit_idx = number_of_trailing_zeros(value);
			value >>= lowest_1_bit_idx;

			step_count_evn += lowest_1_bit_idx;
		}

		iter_count++;
	}

	bool not_finished() {
		return value != 1;
	}
};

#endif /* COLLATZ_CHECKER_NAIVE_H_ */
