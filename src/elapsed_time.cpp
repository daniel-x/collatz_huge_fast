#include "elapsed_time.h"
#include <sstream>
#include <iomanip>

using std::ostream;
using std::ostringstream;
using std::string;

namespace elapsed_time {

string format_time(elapsed_time_ns time_point_ns) {
	std::time_t t = (time_point_ns + NS_PER_SEC / 2) / NS_PER_SEC;

	ostringstream oss;
	oss << std::put_time(std::gmtime(&t), "%Y-%m-%d_%H:%M:%S");
	return oss.str();
}

string format_time_ms(elapsed_time_ns time_point_ns) {
	std::time_t t = (time_point_ns + NS_PER_SEC / 2) / NS_PER_SEC;

	ostringstream oss;
	oss << std::put_time(std::gmtime(&t), "%Y-%m-%d_%H:%M:%S");

	auto t_ms = t % 1000;

	oss << '.' << std::setfill('0') << std::setw(3) << t_ms;

	return oss.str();
}

static ostream& format_with_3_decimal_places(ostream &os, elapsed_time_ns d) {
	auto whole = d / 1000;
	auto rem = abs(d % 1000);

	os << whole << '.' << std::setfill('0') << std::setw(3) << rem;

	return os;
}

string format_dura_s(elapsed_time_ns duration) {
	ostringstream os;

	duration /= 1000000;

	format_with_3_decimal_places(os, duration);

	return os.str();
}

string format_dura(elapsed_time_ns duration) {
	ostringstream oss;

	if (duration < 0) {
		duration = -duration;
		oss << '-';
	}

	if (duration < 1000) {
		// less than a microsecond, so output in ns
		oss << duration << "ns";

	} else if (duration < 1000 * 1000) {
		// less than a millisecond, so output in us
		format_with_3_decimal_places(oss, duration) << "us";

	} else if ((duration + 500) < NS_PER_SEC) {
		// less than a second, so output in ms
		duration += 500;
		elapsed_time_ns duration_us = duration / 1000;
		format_with_3_decimal_places(oss, duration_us) << "ms";

	} else if ((duration + 500 * 1000) < NS_PER_SEC * 60) {
		// less than a minute, so output in s
		duration += 500 * 1000;
		elapsed_time_ns duration_ms = duration / (1000 * 1000);
		format_with_3_decimal_places(oss, duration_ms) << "s";

	} else if ((duration + 500 * 1000 * 100) < NS_PER_SEC * 60 * 60) {
		// less than an hour
		duration += 500 * 1000 * 100;
		auto duration_sec10th = duration / (1000 * 1000 * 100);

		auto m = (duration_sec10th / 10) / 60;
		auto s = (duration_sec10th / 10) % 60;
		auto sec10th = duration_sec10th % 10;

		oss << std::setfill('0') << std::setw(2) << m << 'm';
		oss << std::setfill('0') << std::setw(2) << s << '.' << sec10th << 's';

	} else if ((duration + NS_PER_SEC / 2) < NS_PER_SEC * 60 * 60 * 24) {
		// less than a day
		auto duration_s = (duration + NS_PER_SEC / 2) / NS_PER_SEC;

		auto h = (duration_s / (60 * 60));
		auto m = (duration_s / 60) % 60;
		auto s = duration_s % 60;

		oss << std::setfill('0') << std::setw(2) << h << 'h';
		oss << std::setfill('0') << std::setw(2) << m << 'm';
		oss << std::setfill('0') << std::setw(2) << s << 's';

	} else if ((duration + NS_PER_SEC / 2) < NS_PER_SEC * 60 * 60 * 24 * DAYS_PER_YEAR) {
		// less than a year
		auto duration_s = (duration + NS_PER_SEC / 2) / NS_PER_SEC;

		auto d = (duration_s / (60 * 60 * 24));
		auto h = (duration_s / (60 * 60)) % 24;
		auto m = (duration_s / 60) % 60;
		auto s = duration_s % 60;

		oss << d << "d_";
		oss << std::setfill('0') << std::setw(2) << h << 'h';
		oss << std::setfill('0') << std::setw(2) << m << 'm';
		oss << std::setfill('0') << std::setw(2) << s << 's';

	} else {
		// a year or more
		auto duration_s = (duration + NS_PER_SEC / 2) / NS_PER_SEC;

		auto y = (duration_s / (60 * 60 * 24) * (100 * 1000 * 1000) / DAYS_PER_YEAR_MUL_100M);
		duration_s -= y * 60 * 60 * 24 * DAYS_PER_YEAR_MUL_100M / (100 * 1000 * 1000);
		auto d = (duration_s / (60 * 60 * 24));
		auto h = (duration_s / (60 * 60)) % 24;
		auto m = (duration_s / 60) % 60;
		auto s = duration_s % 60;

		oss << y << "y_";
		oss << d << "d_";
		oss << std::setfill('0') << std::setw(2) << h << 'h';
		oss << std::setfill('0') << std::setw(2) << m << 'm';
		oss << std::setfill('0') << std::setw(2) << s << 's';
	}

	return oss.str();
}

string dura_since(elapsed_time_ns start_time) {
	auto curr_time = system_time();
	string result = format_dura(curr_time - start_time);
	return result;
}

} /* namespace elapsed_time */

