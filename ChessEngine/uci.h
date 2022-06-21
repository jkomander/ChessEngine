#pragma once

#include<thread>

#include"search.h"

namespace uci {
	inline Board board = Board();
	inline Search search = Search();
	inline std::thread searchThread;

	void position(istringstream& ss);
	void go(const string& str);
	void loop();
	Move toMove(const string& str);

	void searchAndPrint();
}