#include "power_of_3_big.h"

#include <gmp.h>

using std::vector;

namespace power_of_3_big {

vector<mpz_class> create() {
	vector<mpz_class> t(LOOKUP_TABLE_INITIAL_SIZE);

	t[0] = 1;

	for (size_t i = 1; i < t.size(); i++) {
		mpz_mul_ui(t[i].get_mpz_t(), t[i - 1].get_mpz_t(), 3);
	}

	return t;
}

vector<mpz_class> LOOKUP_TABLE = create();

} /* namespace power_of_3_big */
