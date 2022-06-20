#include<iostream>

#include"movegen.h"
#include"uci.h"

void uci::position(Board& board, istringstream& ss) {
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
		move = uci::toMove(board, token);
		
		// invalid move
		if (move == Move()) {
			cout << "Invalid move: " << token << "\n";
			break;
		}

		board.applyMove(move);
	}
}

void uci::go(Board& board, const string& str) {
	istringstream ss(str);
	Search search = Search();
	Move best = search.bestMove(board);

	cout << "bestmove " << best.toString() << "\n";
}

void uci::runLoop() {
	Board board;
	string line, token;
	istringstream ss;
	for (;;) {
		getline(cin, line);
		ss = istringstream(line);
		ss >> token;

		if (token == "quit")
			break;

		else if (token == "id") {
			cout << "id name MyChessEngine" << "\n";
			cout << "id author Me" << "\n";
		}
		else if (token == "uci")
			cout << "uciok\n";
		else if (token == "isready")
			cout << "readyok\n";
		else if (token == "position") position(board, ss);
		else if (token == "go")       go(board, ss.str());
		else if (token == "d") cout << board << "\n";
		
	}
}

Move uci::toMove(Board& board, const string& str) {
	MoveList moves = generateMoves(board);
	for (auto& m : moves) {
		if (m.toString() == str)
			return m;
	}
	return Move();
}