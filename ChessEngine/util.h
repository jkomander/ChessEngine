#pragma once

#include<cassert>
#include<chrono>	
#include<iostream>

struct Stopwatch {
	std::chrono::time_point<std::chrono::steady_clock> begin;
	std::chrono::time_point<std::chrono::steady_clock> end;
	uint64_t elapsed;

	void start() {
		begin = std::chrono::steady_clock::now();
	}

	void stop() {
		end = std::chrono::steady_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
	}

	void print() {
		std::cout << "Elapsed time: " << elapsed * 1e-9 << "\n";
	}
};