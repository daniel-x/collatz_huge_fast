#ifndef COLLATZ_CHECKER_FAST_H_
#define COLLATZ_CHECKER_FAST_H_

#include <bits/move.h>
#include <bits/stdint-intn.h>
#include <gmp.h>
#include <gmpxx.h>
#include <stddef.h>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "collatz_multistep.h"
#include "mpz_utils.h"
#include "power_of_3_big.h"

typedef unsigned __int128 dbl_limb_t;

inline mpz_class& operator+=(mpz_class &lhs, const dbl_limb_t &rhs) {
	if (sizeof(dbl_limb_t) != 2 * sizeof(mp_limb_t)) {
		throw std::runtime_error("sizeof(dbl_limb_t) is not double of sizeof(mp_limb_t)");
	}

	mpz_t rhs_mpz;
	mpz_roinit_n(rhs_mpz, reinterpret_cast<const mp_limb_t*>(&rhs), 2);

	mpz_add(lhs.get_mpz_t(), lhs.get_mpz_t(), rhs_mpz);

	return lhs;
}

inline mpz_class operator+(mpz_class lhs, const dbl_limb_t &rhs) {
	lhs += rhs;
	return lhs;
}

// arith_buffer
// accumulator
// accu_chain

// a buffer that can do calculations on its content
class arith_buffer {
public:
	mpz_class value = 0;
	size_t available = 0;

	inline void reset() {
		value = 0;
		available = 0;
	}

	// swap two instances' contents efficiently in O(1)
	inline void swap(arith_buffer &other) {
		mpz_swap(value.get_mpz_t(), other.value.get_mpz_t());
		std::swap(available, other.available);
	}

	inline void adjust_available_to_value() {
		available = size(value);
	}

	inline bool empty() {
		return available == 0 && value == 0;
	}

	inline void ensure_available(size_t expected_available) {
		if (available < expected_available) {
			std::ostringstream os;
			os << "not enough data available (available=" << available << ", expected_available=" << expected_available
					<< ")";
			throw std::runtime_error(os.str());
		}
	}

	inline void pop_back(size_t size, mpz_class &result) {
		ensure_available(size);

		mpz_tdiv_r_2exp(result.get_mpz_t(), value.get_mpz_t(), size * LIMB_BITSIZE);

		value >>= size * LIMB_BITSIZE;

		available -= size;
	}

	inline mp_limb_t pop_back() {
		ensure_available(1);

		mp_limb_t result = value.get_ui();

		value >>= LIMB_BITSIZE;

		available--;

		return result;
	}

	template<typename LARGEINT_OR_BIGINT_TYPE>
	inline void push_back(const LARGEINT_OR_BIGINT_TYPE &pushed_value, size_t pushed_available) {
		value <<= pushed_available * LIMB_BITSIZE;

		value += pushed_value;

		available += pushed_available;
	}

	inline void push_front(const mpz_class &pushed_value, size_t pushed_available) {
		value += pushed_value << (available * LIMB_BITSIZE);

		available += pushed_available;
	}
};

class accumulator {
public:
	arith_buffer buf;
	mp_limb_t exp_of_3 = 0;

	inline void reset() {
		exp_of_3 = 0;
		buf.reset();
	}

	inline void adjust_available_to_value() {
		buf.adjust_available_to_value();
	}

	// swap two instances' contents efficiently in O(1)
	inline void swap(accumulator &other) {
		buf.swap(other.buf);
		std::swap(exp_of_3, other.exp_of_3);
	}

	inline void pop_back(size_t size, mpz_class &result) {
		buf.pop_back(size, result);
	}

	inline mp_limb_t pop_back() {
		return buf.pop_back();
	}

	template<typename LARGEINT_OR_BIGINT_TYPE>
	inline void push_back(const LARGEINT_OR_BIGINT_TYPE &pushed_value, size_t pushed_exp_of_3,
			size_t pushed_available) {
		if (pushed_exp_of_3 < power_of_3_big::LOOKUP_TABLE_INITIAL_SIZE) {
			const mpz_class &pow_of_3 = power_of_3_big::LOOKUP_TABLE[pushed_exp_of_3];
			buf.value *= pow_of_3;
		} else {
			const mpz_class pow_of_3 = power_of_3_big::calculate(pushed_exp_of_3);
			buf.value *= pow_of_3;
		}

		buf.push_back(pushed_value, pushed_available);

		exp_of_3 += pushed_exp_of_3;
	}

	inline bool empty() {
		return buf.empty();
	}

	inline void ensure_available(size_t expected_available) {
		buf.ensure_available(expected_available);
	}

	void push_to_parent(accumulator &parent) {
		parent.push_back(buf.value, exp_of_3, buf.available);

		exp_of_3 = 0;
		buf.value = 0;
		buf.available = 0;
	}

	void pull_from_parent(accumulator &parent, size_t pull_size) {
		size_t actual_pull_size = std::min(pull_size, parent.buf.available);

		mpz_class pulled_value;
		parent.pop_back(actual_pull_size, pulled_value);

		if (exp_of_3 < power_of_3_big::LOOKUP_TABLE_INITIAL_SIZE) {
			const mpz_class &pow_of_3 = power_of_3_big::LOOKUP_TABLE[exp_of_3];
			pulled_value *= pow_of_3;
		} else {
			const mpz_class pow_of_3 = power_of_3_big::calculate(exp_of_3);
			pulled_value *= pow_of_3;
		}

		buf.push_front(pulled_value, pull_size);
	}
};

class accu_chain {
public:
	// size to be pulled from accu_list[idx] into its child accu_list[idx - 1]
	static inline constexpr size_t get_pull_size(size_t idx) {
		return ((size_t) 1) << (idx + 1);
	}

	// value size of accu_list[idx] at which to trigger a push to its parent accu_list[idx + 1]
	static inline constexpr size_t get_push_trigger_value_size(size_t idx) {
		return get_pull_size(idx) * 7 / 5;
	}

	static inline constexpr int64_t ceil_constexpr(double d) {
		int64_t i = static_cast<int64_t>(d);

		i = (static_cast<double>(i) == d) ? i : i + ((d > 0.0) ? 1 : 0);

		return i;
	}

	// exp_of_3 of accu_list[idx] at which to trigger a push to its parent accu_list[idx + 1]
	static inline constexpr size_t get_push_trigger_exp_of_3(size_t idx) {
		const double LOG_BASE2_OF_3 = 1.58496250072115618145;

		double result = get_push_trigger_value_size(idx) * LIMB_BITSIZE / LOG_BASE2_OF_3;

		return (size_t) ceil_constexpr(result);
	}

	// chained accumulators; this list always contains at least one element.
	std::vector<accumulator> accu_list;

	accu_chain() {
		accu_list.push_back(accumulator());
	}

	inline void reset() {
		while (accu_list.size() > 1) {
			accu_list.pop_back();
		}

		accu_list[0].reset();
	}

	inline bool empty() {
		for (auto acc : accu_list) {
			if (!acc.empty()) {
				return false;
			}
		}

		return true;
	}

	std::string str() {
		std::ostringstream os;

		os << "" //
				<< "level" << "\t" //
				<< "availbl" << "\t" //
				<< "val_sz" << "\t" //
				<< "tr_sz" << "\t" //
				<< "exp3" << "\t" //
				<< "tr_exp3" << "\t" //
				<< "pull_sz" << "\n" //
				;

		for (size_t i = accu_list.size() - 1; i < accu_list.size(); i--) {
			accumulator &acc = accu_list[i];
			os << "" //
					<< "[" << std::setw(2) << i << "]\t" // level of accu
					<< acc.buf.available << "\t" // number of limbs available (can be more than saved because of leading zeros)
					<< size(acc.buf.value) << "\t" // number of limbs stored
					<< get_push_trigger_value_size(i) << "\t" // trigger based on size when to push upwards
					<< acc.exp_of_3 << "\t" // exponent of delayed *3^exponent
					<< get_push_trigger_exp_of_3(i) << "\t" // trigger based on exp_of_3 when to push upwards
					<< get_pull_size(i) << "\n" // size ofa pull from this accu
					;

		}

		return os.str();
	}

	inline bool prepare_pop_back() {
		if (accu_list.empty()) {
			throw std::runtime_error("bug. accu_list may not be empty");
		}

		if (accu_list[0].buf.available >= 1) {
			return true;
		}

		if (accu_list.size() == 1) {
			return !accu_list.front().empty();
		}

		for (size_t i = 0; i + 2 < accu_list.size(); i++) {

			// size to be pulled from accu_list[idx] into its child accu_list[idx - 1]
			size_t pull_size = get_pull_size(i);

			if (accu_list[i + 1].buf.available >= pull_size) {
				chained_pull(i);
				return true;
			}
		}

		// no accu has complete pull_size data, so pull anything that's left
		if (accu_list.back().buf.available == 0) {
			throw std::runtime_error("bug. accu_list may not be in this state at this line.");
		}

		chained_pull(accu_list.size() - 2);
		return true;
	}

	inline mp_limb_t pop_back() {
		ensure_available(1);

		return accu_list[0].pop_back();
	}

	inline bool is_push_trigger_reached(size_t idx) {
		bool value_size_reached = is_push_trigger_value_size_reached(idx);
		bool exp_of_3_reached = is_push_trigger_exp_of_3_reached(idx);

		return value_size_reached || exp_of_3_reached;
	}

	inline bool is_push_trigger_value_size_reached(size_t idx) {
		bool result = size(accu_list[idx].buf.value) > get_push_trigger_value_size(idx);
		return result;
	}

	inline bool is_push_trigger_exp_of_3_reached(size_t idx) {
		bool result = accu_list[idx].exp_of_3 > get_push_trigger_exp_of_3(idx);
		return result;
	}

	// inserts a new accumulator into the second-last position
	inline void add_accumulator() {
		accu_list.push_back(accumulator());
		accu_list.end()[-2].swap(accu_list.end()[-1]);
	}

	// pushes the specified pushed_value to the back of this accu chain without shift,
	// i.e. aligned with the lowest accumulator
	inline void push_back(const dbl_limb_t &pushed_value, size_t pushed_exp_of_3) {
		if (accu_list.size() == 1) {
			if (!is_push_trigger_value_size_reached(0)) {
				accu_list[0].push_back(pushed_value, pushed_exp_of_3, 0);
				accu_list[0].adjust_available_to_value();
				return;
			}

			add_accumulator();
		}

		accu_list[0].push_back(pushed_value, pushed_exp_of_3, 0);

		for (size_t i = 0;; i++) {
			if (!is_push_trigger_reached(i)) {
				return;
			}

			if (i == accu_list.size() - 2) {
				add_accumulator();
			}

			accu_list[i].push_to_parent(accu_list[i + 1]);
		}
	}

private:
	inline void ensure_available(size_t expected_available) {
		if (expected_available == 0) {
			return;
		}

		if (accu_list.empty()) {
			std::ostringstream os;
			os << "no more accumulators, so no data available (expected_available=" << expected_available << ")";
			throw std::runtime_error(os.str());
		}

		if (accu_list[0].buf.available < expected_available) {
			std::ostringstream os;
			os << "not enough data available in accu_list[0] (accu_list[0].available=" << accu_list[0].buf.available
					<< ", expected_available=" << expected_available << ")";
			throw std::runtime_error(os.str());
		}
	}

	// with s := i_start, pull [s+1]->[s]->[s-1]->[s-2]->...->[0]
	inline void chained_pull(size_t i_start) {
		for (size_t i = i_start; i + 1 >= 1; i--) {
			size_t pull_size = get_pull_size(i);
			accu_list[i].pull_from_parent(accu_list[i + 1], pull_size);
		}

		if (i_start == accu_list.size() - 2) {
			while (accu_list.size() > 1 && accu_list.back().empty()) {
				accu_list.pop_back();

				accu_list.back().buf.adjust_available_to_value();
			}
		}
	}
};

class collatz_checker_fast {
public:
	accu_chain chain;

	size_t step_count_evn = 0;
	size_t step_count_odd = 0;
	size_t iter_count = 0;

	collatz_checker_fast() {
	}

	std::string str() {
		std::ostringstream os;

		os << "" //
				<< "iter_count....: " << iter_count << "\n" //
				<< "step_count_evn: " << step_count_evn << "\n" //
				<< "step_count_odd: " << step_count_odd << "\n" //
				<< "step_count....: " << step_count() << "\n" //
				;

		os << chain.str();

		return os.str();
	}

	mpz_class& start_value_ref() {
		return chain.accu_list[0].buf.value;
	}

	void start_value_modified() {
		chain.accu_list[0].buf.adjust_available_to_value();
	}

	size_t step_count() {
		return step_count_evn + step_count_odd;
	}

	inline void reset() {
		step_count_evn = 0;
		step_count_odd = 0;
		iter_count = 0;

		chain.reset();
	}

	std::string type_abbrev() {
		return "fast";
	}

	static bool contains(std::vector<size_t> &vec, size_t element) {
		return std::find(vec.begin(), vec.end(), element) != vec.end();
	}

	void complete_check() {
		std::vector<size_t> interesting = { };
		// std::vector<size_t> interesting = { 20 };
		// std::vector<size_t> interesting = { 243227, 243228, 243229, 243230, 243231, 243232, 243233, 243234 };

		bool debug = contains(interesting, iter_count);

		while (chain.prepare_pop_back()) {
			if (debug) {
				std::cout << "after prepare_pop_back:\n" << str();
			}

			iterate();

			debug = contains(interesting, iter_count);

			if (debug) {
				std::cout << "after iterate:\n" << str();
				std::cout << "";
			}
		}
	}

//	inline void iterate() {
//		dbl_limb_t sub_accu = chain.pop_back();
//
//		size_t exponent = 0;
//		if (!chain.empty()) {
//			collatz_multistep::simple_exactly<decltype(sub_accu), LIMB_BITSIZE>(sub_accu, step_count_evn, exponent);
//			step_count_odd += exponent;
//
//		} else {
//			collatz_multistep::simple_at_most<decltype(sub_accu), LIMB_BITSIZE>(sub_accu, step_count_evn, exponent);
//			step_count_odd += exponent;
//
//			if (sub_accu == 1) {
//				iter_count++;
//				return;
//			}
//		}
//
//		chain.push_back(sub_accu, exponent);
//
//		iter_count++;
//	}

	void iterate() {
		dbl_limb_t sub_accu = chain.pop_back();

		size_t exponent;

		exponent = 0;
		if (!chain.empty()) {
			collatz_multistep::combined_impact_exactly<decltype(sub_accu), LIMB_BITSIZE>(sub_accu, step_count_evn,
					exponent);
			step_count_odd += exponent;
		} else {
			collatz_multistep::simple_at_most<decltype(sub_accu), LIMB_BITSIZE>(sub_accu, step_count_evn, exponent);
			step_count_odd += exponent;

			if (sub_accu == 1) {
				iter_count++;
				return;
			}
		}

		chain.push_back(sub_accu, exponent);

		iter_count++;
	}
};

#endif /* COLLATZ_CHECKER_FAST_H_ */
