#include"timeman.h"

TimePoint TimeManagement::now() {
	return std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::steady_clock::now().time_since_epoch()).count();
}

void TimeManagement::start() {
	begin = now();
}

TimePoint TimeManagement::elapsed() {
	return now() - begin;
}

bool TimeManagement::hitTimeLimit() {
	return elapsed() >= optimum;
}