#pragma once

#include<chrono>

using TimePoint = std::chrono::milliseconds::rep;

struct TimeManagement {
	TimePoint begin;
	TimePoint optimum;
	TimePoint maximum;
	bool stop;

	void start();
	TimePoint elapsed();
	static TimePoint now();
	bool hitTimeLimit();
};