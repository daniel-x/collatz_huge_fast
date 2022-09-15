#ifndef POWER_OF_3_INT_H_
#define POWER_OF_3_INT_H_

#include <stddef.h>
#include <array>
#include <limits>

namespace power_of_3_int {

// returns max_fit so that all n <= max_fit will fit after *3 and all
// larger values will overflow by *3.
template<typename INT_TYPE>
constexpr inline INT_TYPE max_fit_for_mul3() {
	INT_TYPE limit = std::numeric_limits<INT_TYPE>::max();

	// 2²ⁿ-1 is always divisible by 3, so limit is divisible by 3,
	// if limit is a typical unsigned bounded integer data type.

	// 2²ⁿ⁺¹-1 % 3 = 1, so max-1 is divisible by 3, if max is a
	// typical signed bounded integer data types. However, we would round
	// down anyways, so we can simply use max / 3 in this case as well.

	return limit / 3;
}

template<typename INT_TYPE>
constexpr size_t max_exponent() {
	INT_TYPE pow = 1;
	size_t exponent = 0;

	while (pow <= max_fit_for_mul3<INT_TYPE>()) {
		pow *= 3;
		exponent++;
	}

	return exponent;
}

template<typename INT_TYPE>
constexpr INT_TYPE calculate(size_t exponent) {
	if (exponent > max_exponent<INT_TYPE>()) {
		throw std::runtime_error("exponent too large to hold the power 3^exponent in this type");
	}

	INT_TYPE pow3 = 1;
	for (size_t i = 0; i < exponent; i++) {
		pow3 *= 3;
	}

	return pow3;
}

template<typename INT_TYPE>
auto create() {
	const size_t SIZE = max_exponent<INT_TYPE>() + 1;

	std::array<INT_TYPE, SIZE> result;

	result[0] = 1;

	for (size_t i = 1; i < SIZE; i++) {
		result[i] = result[i - 1] * 3;
	}

	return result;
}

template<typename INT_TYPE>
const auto LOOKUP_TABLE = create<INT_TYPE>();

} /* namespace power_of_3_int */

#endif /* POWER_OF_3_INT_H_ */
