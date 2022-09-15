#ifndef POWER_OF_3_BIG_H_
#define POWER_OF_3_BIG_H_

#include <gmpxx.h>
#include <stddef.h>
#include <memory>
#include <vector>

namespace power_of_3_big {

extern std::vector<mpz_class> LOOKUP_TABLE;

const size_t LOOKUP_TABLE_INITIAL_SIZE = (1 << 17) + 1;

inline mpz_class calculate(size_t exponent) {
	mpz_class pow3;

	mpz_ui_pow_ui(pow3.get_mpz_t(), 3, exponent);

	return pow3;
}

} /* namespace power_of_3_big */

#endif /* POWER_OF_3_BIG_H_ */
