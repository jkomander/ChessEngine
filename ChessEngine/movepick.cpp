#include"movepick.h"

MovePicker::MovePicker(Search* search, Board* board, Move hashMove, Move killerMove1, Move killerMove2) {
	std::fill(scores, scores + 256, 0);
	this->hashMove = hashMove;
	this->killerMove1 = killerMove1;
	this->killerMove2 = killerMove2;
	stage = HASHMOVE;
	idx = 0;
	this->search = search;
	this->board = board;
}

Move MovePicker::next() {
	switch (stage) {
	case HASHMOVE: {
		++stage;
		
		if (hashMove)
			return hashMove;
	}

	case KILLER1:
		++stage;

		if (killerMove1 && killerMove1 != hashMove)
			return killerMove1;

	case KILLER2:
		++stage;

		if (killerMove2 && killerMove2 != hashMove)
			return killerMove2;

	case NORMAL_INIT: {
		++stage;

		moves = generateMoves(*board);

		if (hashMove) {
			auto it = std::find(moves.begin(), moves.end(), hashMove);
			// assert that hashMove is legal
			if (it == moves.end()) {
				print(moves);
				std::cout << hashMove << "\n" << * board->st << board << "\n";
			}
			assert(it != moves.end());
			moves.erase(it);
		}

		if (killerMove1) {
			auto it = std::find(moves.begin(), moves.end(), killerMove1);
			if (it != moves.end())
				moves.erase(it);
		}

		if (killerMove2) {
			auto it = std::find(moves.begin(), moves.end(), killerMove2);
			if (it != moves.end())
				moves.erase(it);
		}

		std::shuffle(moves.begin(), moves.end(), random::rng);

		// history heuristic
		std::stable_sort(moves.begin(), moves.end(),
			[&](Move m1, Move m2) {
				return search->history[board->getPiece(m1.from)][m1.to] >
					search->history[board->getPiece(m2.from)][m2.to];
			}
		);
	}

	case NORMAL:
		if (idx >= moves.size())
			return Move();
		++idx;
		return moves[idx - 1];
	}
}