#include<iostream>

#include"movegen.h"
#include"uci.h"

void uci::position(istringstream& ss) {
	string token, fen;
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
			cout << "Invalid move: " << token << "\n";
			break;
		}

		board.applyMove(move);
	}
}

void uci::go(const string& str) {
	istringstream ss(str);

	// start the search
	searchThread = std::thread(searchAndPrint);
}

void uci::loop() {
	string line, token;
	istringstream ss;
	for (;;) {
		getline(cin, line);
		ss = istringstream(line);
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
			cout << "uciok\n";
		else if (token == "isready")
			cout << "readyok\n";
		else if (token == "position") position(ss);
		else if (token == "go")       go(ss.str());

		// non-UCI commands
		else if (token == "d") cout << board << "\n";
	}
}

Move uci::toMove(const string& str) {
	MoveList moves = generateMoves(board);
	for (auto& m : moves) {
		if (m.toString() == str)
			return m;
	}
	return Move();
}

void uci::searchAndPrint() {
	Move best = search.bestMove(board);
	cout << "bestmove " << best.toString() << "\n";
}