#ifndef ELAPSED_TIME_H_
#define ELAPSED_TIME_H_

#include <chrono>
#include <string>

/*
 * This version uses a signed 64 bit integer to represent points in time. The
 * maximum value of this integer is 2^63-1 and therefore the min/max time it can
 * represent is
 * 1970-01-01 00:00 +/- (2^63)ns
 * = 1970-01-01 00:00 +/- (2^63)ns / (1000 * 1000 * 1000 * 60 * 60 * 24 * 365.242191)ns/y
 * = 1970-01-01 00:00 +/- (2^63)ns / (1000 * 1000 * 1000 * 60 * 60 * 24 * 365.242191)ns/y
 *
 * 292.27727189737684470946
 *
 */

namespace elapsed_time {

/*
 * An integer specifying either a point in time as nano seconds since epoch
 * (1970-01-01 00:00 UTC) or a time difference, i.e. a duration, between two
 * points in time.
 */
typedef long long int elapsed_time_ns;

/*
 * Days per year 365.24219052 * 100'000'000, rounded to integer. This value
 * cannot be exact, because the period of the earth roaming around the sun and
 * the period of the earth spinning around its own axis don't line up.
 */
static const elapsed_time_ns DAYS_PER_YEAR_MUL_100M = 36524219052;

static const double DAYS_PER_YEAR = ((double) DAYS_PER_YEAR_MUL_100M) / 100000000;

/*
 * Nano seconds in one second, i.e. 10^9.
 */
static const elapsed_time_ns NS_PER_SEC = 1000ul * 1000 * 1000;

inline elapsed_time_ns system_time() {
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

inline elapsed_time_ns steady_time() {
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

inline elapsed_time_ns highres_time() {
	return std::chrono::duration_cast<std::chrono::nanoseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

std::string format_time(elapsed_time_ns time_point_ns);
std::string format_time_ms(elapsed_time_ns time_point_ns);
std::string format_dura_s(elapsed_time_ns duration_ns);

/*
 * Prints the duration since the specified start_time, as measured by
 * system_time(). This is a convenience function, which does the call to
 * system_time() for you and calculates the duration since start_time.
 */
std::string dura_since(elapsed_time_ns start_time);

/*
 * Takes a nanosecond duration and formats it to a human readable string by
 * converting it to useful time units.<br/>
 * You can use differences of two calls to one of the time functions and feed it into this function.<br/>
 * <br/>
 * Time units are:<br/>
 * <ul>
 * <li>ps (picosecond, 0.000000000001s)</li>
 * <li>ns (nanosecond, 0.000000001s)</li>
 * <li>us (microsecond, 0.000001s, like µs, but more compatible as us)</li>
 * <li>ms (mllisecond, 0.001s)</li>
 * <li>s (second, 1s)</li>
 * <li>m (minute, 60s)</li>
 * <li>h (hour, 3600s)</li>
 * <li>d (day, 86400s)</li>
 * <li>y (year, 86400s * 365.242191)</li>
 * </ul>
 */
std::string format_dura(elapsed_time_ns duration_ns);

} /* namespace elapsed_time */

#endif /* ELAPSED_TIME_H_ */
