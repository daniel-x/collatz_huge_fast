/*
 * amount_formatter.cpp   2020-08-25
 *
 * Copyright 2020 Daniel Strecker
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "amount_formatter.h"
#include <cmath>
#include <string>
#include <sstream>

using std::string;
using std::ostringstream;

namespace amount_formatter {

static long long int K = 1000;
static long long int M = K * K;
static long long int G = M * K;
static long long int T = G * K;
static long long int P = T * K;
static long long int E = P * K;
static long long int Z = E * K;
static long long int Y = Z * K;
static long long int INTERVAL_LIST_METRIC[] = { 1, K, M, G, T, P, E, Z, Y };
static string UNIT_LIST_METRIC[] =
		{ "", "k", "M", "G", "T", "P", "E", "Z", "Y" };

static long long int Ki = 1024;
static long long int Mi = Ki * Ki;
static long long int Gi = Mi * Ki;
static long long int Ti = Gi * Ki;
static long long int Pi = Ti * Ki;
static long long int Ei = Pi * Ki;
static long long int Zi = Ei * Ki;
static long long int Yi = Zi * Ki;
static long long int INTERVAL_LIST_KIBI[] =
		{ 1, Ki, Mi, Gi, Ti, Pi, Ei, Zi, Yi };
static string UNIT_LIST_KIBI[] = { "", "Ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi",
		"Yi" };

static int get_interval(long long int amount, long long int interval_list[]) {
	if (amount < interval_list[1]) {
		return 0;
	} else if (amount < interval_list[2]) {
		return 1;
	} else if (amount < interval_list[3]) {
		return 2;
	} else if (amount < interval_list[4]) {
		return 3;
	} else if (amount < interval_list[5]) {
		return 4;
	} else if (amount < interval_list[6]) {
		return 5;
	} else if (amount < interval_list[7]) {
		return 6;
	} else if (amount < interval_list[8]) {
		return 7;
	} else {
		return 8;
	}
}

static string to_string(double value, int precision) {
	ostringstream os;
	os.precision(precision);

	os << std::fixed << value;

	return os.str();
}

static string to_rounded_string(double value) {
	string result;

	if (trunc(value) == value) {
		result = std::to_string((long long int) value);
	} else if (value >= 99.95) {
		result = std::to_string((long long int) std::round(value));
	} else {
		result = to_string(value, 1);
	}

	return result;
}

static string format(long long int amount, long long int interval_list[],
		string unit_list[]) {
	string sign_str;
	if (amount < 0) {
		amount = -amount;
		sign_str = "-";
	} else {
		sign_str = "";
	}

	int interval = get_interval(amount, interval_list);
	double converted = ((double) amount) / interval_list[interval];

	string unit_str = unit_list[interval];

	string converted_str = to_rounded_string(converted);

	return sign_str + converted_str + unit_str;
}

/**
 * Correct metric formatting with 1000 as conversion factor and metric units k,
 * M, and so on. Many storage device producers use this metric format to make
 * their devices look bigger, because everybody thinks they were using the
 * classic conversion factor of 1024, but actually use the metric factor 1000.
 */
string format_metric(long long int amount) {
	return format(amount, INTERVAL_LIST_METRIC, UNIT_LIST_METRIC);
}

/**
 * Format with classic conversion factor of 1024 instead of the metric 1000. To
 * use this correctly on data sizes, use the symbols KiB, MiB, etc together with
 * the output of this function.
 */
string format_kibi(long long int amount) {
	return format(amount, INTERVAL_LIST_KIBI, UNIT_LIST_KIBI);
}

} /* namespace amount_formatter */
