#ifndef COLLATZ_CHECKER_SLOW_H_
#define COLLATZ_CHECKER_SLOW_H_

#include <gmpxx.h>
#include <stddef.h>
#include <string>
#include <vector>
#include <memory>

#include "collatz_multistep.h"
#include "mpz_utils.h"
#include "elapsed_time.h"
namespace ela = elapsed_time;

class collatz_checker_slow {
public:
	mpz_class value = 1;

	size_t step_count_evn = 0;
	size_t step_count_odd = 0;
	size_t iter_count = 0;

	collatz_checker_slow() {
	}

	inline void reset() {
		step_count_evn = 0;
		step_count_odd = 0;
		iter_count = 0;
	}

	inline mpz_class& start_value_ref() {
		return value;
	}

	void start_value_modified() {
	}

	size_t step_count() {
		return step_count_evn + step_count_odd;
	}

	std::string type_abbrev() {
		return "slow";
	}

	void complete_check() {
		while (not_finished()) {
			iterate();
		}
	}

	static const uint_fast8_t LIMB_BITSIZE_HALF = sizeof(mp_limb_t) * 8 / 2;

	static const mp_limb_t LIMB_LO_MASK = ~(((mp_limb_t) -1) << LIMB_BITSIZE_HALF);

	inline void iterate() {
		mp_limb_t lo = value.get_ui();
		value >>= LIMB_BITSIZE;

		dbl_limb_t hi;

		if (value > 0) {
			hi = lo >> LIMB_BITSIZE_HALF;
			lo &= LIMB_LO_MASK;

			size_t exponent_cum = 0;
			size_t exponent;

			mp_limb_t pow_of_3;

			// combined_impact_exactly
			// simple_exactly

			exponent = 0;
			collatz_multistep::combined_impact_exactly<decltype(lo), LIMB_BITSIZE_HALF>(lo, step_count_evn, exponent);
			exponent_cum += exponent;

			pow_of_3 = power_of_3_int::LOOKUP_TABLE<mp_limb_t>[exponent];
			hi *= pow_of_3;
			hi += lo;

			lo = hi & LIMB_LO_MASK;
			hi >>= LIMB_BITSIZE_HALF;

			exponent = 0;
			collatz_multistep::combined_impact_exactly<decltype(lo), LIMB_BITSIZE_HALF>(lo, step_count_evn, exponent);
			exponent_cum += exponent;

			pow_of_3 = power_of_3_int::LOOKUP_TABLE<mp_limb_t>[exponent];
			hi *= pow_of_3;
			hi += lo;

			step_count_odd += exponent_cum;

			if (exponent_cum < power_of_3_big::LOOKUP_TABLE_INITIAL_SIZE) {
				const mpz_class &pow3 = power_of_3_big::LOOKUP_TABLE[exponent_cum];
				value *= pow3;
			} else {
				const mpz_class pow3 = power_of_3_big::calculate(exponent_cum);
				value *= pow3;
			}

		} else {
			hi = lo;
			collatz_multistep::simple_at_most<decltype(hi), LIMB_BITSIZE>(hi, step_count_evn, step_count_odd);
		}

		value += hi;

		iter_count++;
	}

	bool not_finished() {
		return value != 1;
	}
};

#endif /* COLLATZ_CHECKER_SLOW_H_ */
