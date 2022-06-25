#include<ostream>

#include"movegen.h"
#include"uci.h"

void uci::position(std::istringstream& ss) {
	std::string token, fen;
	ss >> token;

	if (token == "startpos") {
		fen = startFEN;
		ss >> token;
	}
	else if (token == "fen") {
		while (ss >> token && token != "moves")
			fen += token + " ";
	}
	else
		return;

	board = Board(fen);

	Move move;
	while (ss >> token) {
		move = uci::toMove(token);
		
		// invalid move
		if (move == Move()) {
			std::cout << "Invalid move: " << token << "\n";
			break;
		}

		board.applyMove(move);
	}
}

void uci::go(const std::string& str) {
	std::istringstream ss(str);

	// start the search
	searchThread = std::thread(searchAndPrint);
}

void uci::loop() {
	std::string line, token;
	std::istringstream ss;
	for (;;) {
		std::getline(std::cin, line);
		ss = std::istringstream(line);
		ss >> token;

		if (token == "quit") {
			break;
		}

		if (token == "stop") {
			search.stop();
			searchThread.join();
		}

		else if (token == "id") {
		}
		else if (token == "uci")
			std::cout << "uciok\n";
		else if (token == "isready")
			std::cout << "readyok\n";
		else if (token == "position") position(ss);
		else if (token == "go")       go(ss.str());

		// non-UCI commands
		else if (token == "d") std::cout << board << "\n";
	}
}

Move uci::toMove(const std::string& str) {
	MoveList moves = generateMoves(board);
	for (auto& m : moves) {
		if (m.toString() == str)
			return m;
	}
	return Move();
}

void uci::searchAndPrint() {
	Move best = search.bestMove(board);
	std::cout << "bestmove " << best.toString() << "\n";
}