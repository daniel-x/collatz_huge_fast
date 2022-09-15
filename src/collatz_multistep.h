#ifndef COLLATZ_MULTISTEP_H_
#define COLLATZ_MULTISTEP_H_

#include <gmp.h>
#include <gmpxx.h>
#include <stddef.h>
#include <limits>
#include <array>
#include <bitset>
#include <cmath>
#include <iostream>
#include <iomanip>

#include "power_of_3_int.h"
#include "power_of_3_big.h"

namespace collatz_multistep {

template<typename INT_TYPE>
inline void simple_single_step(INT_TYPE &value, size_t &step_count_odd) {
	uint_fast8_t is_odd = ((uint_fast8_t) value) & ((uint_fast8_t) 1);
	step_count_odd += is_odd;
	value = (value >> 1) + is_odd * value + is_odd;
}

template<typename INT_TYPE, size_t STEP_COUNT>
inline void simple_exactly(INT_TYPE &value, size_t &step_count_evn, size_t &step_count_odd) {
	for (size_t i = 0; i < STEP_COUNT; i++) {
		simple_single_step<INT_TYPE>(value, step_count_odd);
	}

	step_count_evn += STEP_COUNT;
}

template<typename INT_TYPE, size_t STEP_COUNT>
inline void simple_at_most(INT_TYPE &value, size_t &step_count_evn, size_t &step_count_odd) {
	size_t i = 0;

	for (; i < STEP_COUNT; i++) {
		if (value == 1) {
			break;
		}

		simple_single_step(value, step_count_odd);
	}

	step_count_evn += i;
}

inline void simple_single_step(mpz_class &value, size_t &step_count_odd) {
	uint_fast8_t is_odd = ((uint_fast8_t) value.get_ui()) & ((uint_fast8_t) 1);

	if (is_odd) {
		value *= 3;
		value++;
		step_count_odd++;
	}

	value >>= 1;
}

template<size_t STEP_COUNT>
inline void simple_exactly(mpz_class &value, size_t &step_count_evn, size_t &step_count_odd) {
	for (size_t i = 0; i < STEP_COUNT; i++) {
		simple_single_step(value, step_count_odd);
	}

	step_count_evn += STEP_COUNT;
}

template<size_t STEP_COUNT>
inline void simple_at_most(mpz_class &value, size_t &step_count_evn, size_t &step_count_odd) {
	size_t i = 0;

	for (; i < STEP_COUNT; i++) {
		if (value == 1) {
			break;
		}

		simple_single_step(value, step_count_odd);
	}

	step_count_evn += i;
}

// objects of this class are elements of a lookup table for looking up the net
// impact of the last N bits (typically about 8 or 10) during the next N steps
// combined.
class multistep_impact {
public:
	uint16_t carry;
	uint16_t power;
	uint8_t expnt;

	std::string str() const {
		std::ostringstream ostr;

		ostr << "multistep_impact[" << carry << " " << power << " " << ((uint64_t) expnt) << "]";

		return ostr.str();
	}
};

template<size_t STEP_COUNT>
std::array<multistep_impact, 1 << STEP_COUNT> create_combined_impact_table() {
	std::array<multistep_impact, 1 << STEP_COUNT> result;

	const bool LOG = false;

	if (LOG) {
		std::cout << "postfix" << "\t" << "bin(postfix)" << "\t" << "exponet" << "\t" << "carry" << "\t" << "power"
				<< '\n';
	}

	typedef decltype(multistep_impact::power) INT_TYPE_POWER;
	typedef decltype(multistep_impact::carry) INT_TYPE_CARRY;

	for (uint_fast32_t postfix = 0; postfix < result.size(); postfix++) {
		size_t expnt = 0;

		INT_TYPE_CARRY y = postfix;
		for (size_t i = 0; i < STEP_COUNT; i++) {
			uint_fast8_t is_odd = ((uint_fast8_t) y) & ((uint_fast8_t) 1);
			expnt += is_odd;
			y = (y >> 1) + is_odd * y + is_odd;
		}

		INT_TYPE_POWER power = power_of_3_int::calculate<INT_TYPE_POWER>(expnt);

		result[postfix].carry = y;
		result[postfix].expnt = expnt;
		result[postfix].power = power;

		if (LOG) {
			std::cout << postfix << "\t" << std::bitset<STEP_COUNT>(postfix) << "\t" << expnt << "\t" << y << "\t"
					<< power << '\n';
		}
	}

	return result;
}

const size_t COMBINED_IMPACT_TABLE_STEP_COUNT = 8;
const mp_limb_t COMBINED_IMPACT_MASK = ~(((mp_limb_t) -1) << COMBINED_IMPACT_TABLE_STEP_COUNT);

const auto COMBINED_IMPACT_TABLE = create_combined_impact_table<COMBINED_IMPACT_TABLE_STEP_COUNT>();

template<typename INT_TYPE, size_t STEP_COUNT>
inline void combined_impact_exactly(INT_TYPE &value, size_t &step_count_evn, size_t &step_count_odd) {
	if ((STEP_COUNT % COMBINED_IMPACT_TABLE_STEP_COUNT) != 0) {
		throw std::runtime_error("(STEP_COUNT % COMBINED_IMPACT_TABLE_STEP_COUNT) != 0 not supported");
	}

	const size_t COMBINED_STEP_COUNT = STEP_COUNT / COMBINED_IMPACT_TABLE_STEP_COUNT;

	for (size_t i = 0; i < COMBINED_STEP_COUNT; i++) {
		uint_fast32_t postfix = value & COMBINED_IMPACT_MASK;
		value >>= COMBINED_IMPACT_TABLE_STEP_COUNT;

		step_count_odd += COMBINED_IMPACT_TABLE[postfix].expnt;

		value *= COMBINED_IMPACT_TABLE[postfix].power;

		value += COMBINED_IMPACT_TABLE[postfix].carry;
	}

	step_count_evn += STEP_COUNT;
}

}

#endif /* COLLATZ_MULTISTEP_H_ */
