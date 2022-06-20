#pragma once

#include"search.h"

namespace uci {
	void position(Board& board, istringstream& ss);
	void go(Board& board, const string& str);
	void runLoop();
	Move toMove(Board& board, const string& str);
}