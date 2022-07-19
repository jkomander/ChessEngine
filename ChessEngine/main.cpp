#include<iostream>

#include"attacks.h"
#include"bitboard.h"
#include"board.h"
#include"eval.h"
#include"movegen.h"
#include"search.h"
#include"tt.h"
#include"uci.h"

int main() {
	attacks::init();
	Eval::init();
	Zobrist::init();
	tt.setSize(256);

	uci::loop();
}