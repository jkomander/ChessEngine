#pragma once

#include<thread>

#include"search.h"

namespace uci {
	inline Board board;
	inline Search search = Search();
	inline std::thread searchThread;

	void position(std::istringstream& ss);
	void go(const std::string& str);
	void loop();
	Move toMove(const std::string& str);

	void searchAndPrint();
	void stop();
}