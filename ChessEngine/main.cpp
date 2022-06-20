#include<iostream>

#include"attacks.h"
#include"bitboard.h"
#include"board.h"
#include"eval.h"
#include"movegen.h"
#include"search.h"
#include"tt.h"
#include"uci.h"

#include"util.h"

int main() {
	attacks::init();
	eval::init();
	Zobrist::init();
	tt.setSize(256);

	Board board(startFEN);
	Search search = Search();

	search.bestMove(board);

	for (; board.history.size() > 1;)
		board.undoMove();
	assert(board.fen() == startFEN);
}