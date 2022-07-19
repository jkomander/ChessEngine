#pragma once

#include"movegen.h"
#include"timeman.h"
#include"tt.h"

#include"util.h"

enum SearchType {
	NonPV, PV, Root
};

enum Scores :int16_t {
	DRAW_SCORE = 0,
	INFINITY_SCORE = std::numeric_limits<int16_t>::max(),
	MATE_SCORE = INFINITY_SCORE - 0x100
};

struct Stack {
	Move pv[256];
	int pvSize;
	int ply;
};

struct Search {
	TimeManagement time;
	MoveList pv;
	uint64_t nodes;
	uint64_t history[16][N_SQUARES] = {};
	Move killer[256][2] = {};

	Move bestMove(Board& board);
	template<SearchType searchType>
	Score alphaBeta(Board& board, Stack* ss, Score alpha, Score beta, Depth depth);
	template<SearchType searchType>
	Score quiescence(Board& board, Score alpha, Score beta);

	void stop();
};

inline void Search::stop() {
	time.stop = true;
}