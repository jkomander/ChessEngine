#pragma once

#include<array>
#include<chrono>

#include"bitboard.h"

using TimePoint = std::chrono::milliseconds::rep;

struct TimeManagement {
	TimePoint begin;
	TimePoint optimum;
	TimePoint maximum;
	bool stop;
	std::array<TimePoint, N_COLORS>time;
	std::array<TimePoint, N_COLORS>inc;

	void start();
	TimePoint elapsed();
	static TimePoint now();
	bool hitTimeLimit();
};