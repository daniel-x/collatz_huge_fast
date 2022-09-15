#include <gmp.h>
#include <gmpxx.h>
#include <cmath>
#include <iostream>
#include <vector>

#include "collatz_checker_fast.h"
#include "collatz_checker_slow.h"
#include "collatz_checker_naive.h"
#include "elapsed_time.h"
#include "amount_formatter.h"

using std::cout;
using std::flush;
using std::vector;
namespace ela = elapsed_time;
namespace amf = amount_formatter;

void ensure_matching(uint64_t step_count_evn, uint64_t step_count_evn_expected, uint64_t step_count_odd,
		uint64_t step_count_odd_expected) {

	if (step_count_evn != step_count_evn_expected || step_count_odd != step_count_odd_expected) {
		cout << "" << //
				"step_count_evn: " << step_count_evn << //
				" (expected: " << step_count_evn_expected << ")\n";

		cout << "" << //
				"step_count_odd: " << step_count_odd << //
				" (expected: " << step_count_odd_expected << ")\n";

		throw std::runtime_error("step count incorrect");
	}
}

template<typename CHECKER>
void test_single(const mpz_class &n, uint64_t step_count_evn_expected, uint64_t step_count_odd_expected) {
	CHECKER checker;
	checker.start_value_ref() = n;
	checker.start_value_modified();

	mpz_class n_ = n;

	checker.complete_check();

	ensure_matching(checker.step_count_evn, step_count_evn_expected, checker.step_count_odd, step_count_odd_expected);
}

void test_3_algorithms_consistency() {
	struct test_case {
		mpz_class n;
		uint64_t step_count_evn;
		uint64_t step_count_odd;

		test_case(const char *n_str, uint64_t step_count_evn, uint64_t step_count_odd) {
			this->n = n_str;
			this->step_count_evn = step_count_evn;
			this->step_count_odd = step_count_odd;
		}
	};

	vector<test_case> test_case_list;
	test_case_list.push_back(test_case("3", 5, 2));
	test_case_list.push_back(test_case("765432", 107, 55));
	test_case_list.push_back(test_case("32860794781696", 61, 10));
	test_case_list.push_back(test_case("3287505407311872", 139, 55));
	test_case_list.push_back(test_case("420800692135919616", 146, 55));
	test_case_list.push_back(test_case("970300334233894087246424527897362432", 207, 55));
	test_case_list.push_back(test_case("645643565437415345345235535462318313342346", 605, 294));
	test_case_list.push_back(test_case("156243863292978154974121315437405326167310717681664", 438, 171));
	test_case_list.push_back(test_case("156243863292978154974121315437405326167310717681665", 1034, 547));
	test_case_list.push_back(test_case("7457634543564564356543765868989546221123415345345235", 732, 353));
	test_case_list.push_back(
			test_case(
					"7457634543564564356543765868989546221123264476548153453452351432452366890718900767686634213422312124468578453411236799872211651",
					2013, 1004));

	for (size_t i = 0; i < test_case_list.size(); i++) {
		const auto &test_case = test_case_list[i];

		test_single<collatz_checker_naive>(test_case.n, test_case.step_count_evn, test_case.step_count_odd);
		test_single<collatz_checker_slow>(test_case.n, test_case.step_count_evn, test_case.step_count_odd);
		test_single<collatz_checker_fast>(test_case.n, test_case.step_count_evn, test_case.step_count_odd);
	}
}

void test_very_large_number() {
	mpz_class start_value;

	start_value = 1;
	start_value <<= 1000000;
	start_value++;
	// mpz_ui_pow_ui(start_value.get_mpz_t(), 3, (mp_limb_t) floor(10000000 / LOG_BASE2_OF_3));

	cout << "start value bitlen: " << bitlen(start_value) << "\n\n";

	collatz_checker_slow checker;
	checker.start_value_ref() = start_value;
	checker.start_value_modified();

	cout << "" //
			<< "type" << "\t" //
			<< "step_count_evn" << "\t" //
			<< "step_count_odd" << "\t" //
			<< "step_count_all" << "\t" //
			<< "itrtons" << "\t" //
			<< "runtime" << "\t" //
			<< "runtime_in_s" << //
			"\n";

	cout << checker.type_abbrev() << "\t" << flush;

	ela::elapsed_time_ns t = ela::system_time();

	checker.complete_check();

	t = ela::system_time() - t;

	cout << "" //
			<< checker.step_count_evn << "\t" //
			<< checker.step_count_odd << "\t" //
			<< checker.step_count() << "\t" //
			<< checker.iter_count << "\t" //
			<< ela::format_dura(t) << "\t" //
			<< ela::format_dura_s(t) << //
			"\n";
}

int main() {
	test_3_algorithms_consistency();

	test_very_large_number();

	return 0;
}
