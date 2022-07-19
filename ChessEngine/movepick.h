#pragma once

#include"search.h"

struct MovePicker {
	enum Stage {
		HASHMOVE, KILLER1, KILLER2, NORMAL_INIT, NORMAL
	};

	MoveList moves;
	Score scores[256];
	Move hashMove;
	Move killerMove1;
	Move killerMove2;
	int stage;
	int idx;
	Search* search;
	Board* board;

	MovePicker(Search* search, Board* board, Move hashMove, Move killerMove1, Move killerMove2);

	Move next();
};