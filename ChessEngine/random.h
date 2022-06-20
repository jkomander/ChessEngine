#pragma once

#include<random>

namespace random {
	inline int seed = 0;
	inline std::mt19937_64 rng(seed);
	inline std::normal_distribution<float>normal(0, 0.2);
	
	inline float getFloat() {
		return normal(rng);
	}

	inline float getFloat(uint64_t seed) {
		std::mt19937_64 gen(seed); 
		std::normal_distribution<float>dist(0, 0.2);
		return dist(gen);
	}

	inline uint64_t getULL() {
		return rng();
	}
}