/*
 * amount_formatter.h   2020-08-25
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

/**
 * Functions for formatting amounts, e.g. for data sizes, can format with base
 * 1000 conversion or with base 1024 conversion. Example: 21342553 -> "20.4M".
 * Use the distinguishable format_1024(...) for data sizes together with +"B"
 * and format_metric(...) for everything else.
 */

#ifndef AMOUNT_FORMATTER_H_
#define AMOUNT_FORMATTER_H_

#include <string>

namespace amount_formatter {

/**
 * Correct metric formatting with 1000 as conversion factor and metric units k,
 * M, and so on. Many storage device producers use this metric format to make
 * their devices look bigger, because everybody thinks they were using the
 * classic conversion factor of 1024, but actually use the metric factor 1000.
 */
std::string format_metric(long long int amount);

/**
 * Format with conversion factor of 1024, which is the classic for data sizes.
 */
std::string format_kibi(long long int amount);

}

#endif /* AMOUNT_FORMATTER_H_ */
