#ifndef MPZ_UTILS_H_
#define MPZ_UTILS_H_

#include <gmp.h>
#include <gmpxx.h>
#include <stddef.h>

const size_t LIMB_BITSIZE = sizeof(mp_limb_t) * 8;

inline size_t size(mpz_class &c) {
	return mpz_size(c.get_mpz_t());
}

inline bool is_odd(mpz_class &c) {
	return mpz_odd_p(c.get_mpz_t());
}

inline size_t bitlen(mpz_class &c) {
	return size(c) == 0 ? 0 : mpz_sizeinbase(c.get_mpz_t(), 2);
}

inline size_t number_of_trailing_zeros(mpz_class &c) {
	return mpz_scan1(c.get_mpz_t(), 0);
}

#endif /* MPZ_UTILS_H_ */
