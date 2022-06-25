#pragma once

#include"movegen.h"

namespace eval {
	inline std::array<std::array<Score, N_SQUARES>, N_COLORS>pawnTable;
	inline std::array<std::array<Score, N_SQUARES>, N_COLORS>knightTable;
	inline std::array<std::array<Score, N_SQUARES>, N_COLORS>bishopTable;
	inline std::array<std::array<Score, N_SQUARES>, N_COLORS>rookTable;
	inline std::array<std::array<Score, N_SQUARES>, N_COLORS>queenTable;
	inline std::array<std::array<Score, N_SQUARES>, N_COLORS>mgKingTable;
	inline std::array<std::array<Score, N_SQUARES>, N_COLORS>egKingTable;

	inline std::array<Score, N_PIECE_TYPES>pieceValues = { 100, 350, 350, 525, 1000 };

	inline Score getPieceValue(PieceType pt) {
		return pieceValues[pt - 1];
	}

	inline void init() {
		pawnTable[0] = {
		 0,  0,  0,  0,  0,  0,  0,  0,
		50, 10, 10,-20,-20, 10, 10, 50,
		50,-50,-10,  0,  0,-10,-50, 50,
		 0,  0,  0, 20, 20,  0,  0,  0,
		50, 50, 10, 25, 25, 10, 50, 50,
		10, 10, 20, 30, 30, 20, 10, 10,
		50, 50, 50, 50, 50, 50, 50, 50,
		 0,  0,  0,  0,  0,  0,  0,  0
		};

		knightTable[0] = {
		-50,-40,-30,-30,-30,-30,-40,-50,
		-40,-20,  0,  5,  5,  0,-20,-40,
		-30,  5, 10, 15, 15, 10,  5,-30,
		-30,  0, 15, 20, 20, 15,  0,-30,
		-30,  5, 15, 20, 20, 15,  5,-30,
		-30,  0, 10, 15, 15, 10,  0,-30,
		-40,-20,  0,  0,  0,  0,-20,-40,
		-50,-40,-30,-30,-30,-30,-40,-50
		};

		bishopTable[0] = {
		-20,-10,-10,-10,-10,-10,-10,-20,
		-10,  5,  0,  0,  0,  0,  5,-10,
		-10, 10, 10, 10, 10, 10, 10,-10,
		-10,  0, 10, 10, 10, 10,  0,-10,
		-10,  5,  5, 10, 10,  5,  5,-10,
		-10,  0,  5, 10, 10,  5,  0,-10,
		-10,  0,  0,  0,  0,  0,  0,-10,
		-20,-10,-10,-10,-10,-10,-10,-20
		};

		rookTable[0] = {
		0,  0,  0,  5,  5,  0,  0,  0,
	   -5,  0,  0,  0,  0,  0,  0, -5,
	   -5,  0,  0,  0,  0,  0,  0, -5,
	   -5,  0,  0,  0,  0,  0,  0, -5,
	   -5,  0,  0,  0,  0,  0,  0, -5,
       -5,  0,  0,  0,  0,  0,  0, -5,
	    5, 10, 10, 10, 10, 10, 10,  5,
		0,  0,  0,  0,  0,  0,  0,  0
		};

		queenTable[0] = {
		-20,-10,-10, -5, -5,-10,-10,-20,
		-10,  0,  5,  0,  0,  0,  0,-10,
		-10,  5,  5,  5,  5,  5,  0,-10,
		  0,  0,  5,  5,  5,  5,  0, -5,
	     -5,  0,  5,  5,  5,  5,  0, -5,
		-10,  0,  5,  5,  5,  5,  0,-10,
		-10,  0,  0,  0,  0,  0,  0,-10,
		-20,-10,-10, -5, -5,-10,-10,-20
		};

		mgKingTable[0] = {
		 20, 30, 10,  0,  0, 10, 30, 20,
		 20, 20,  0,  0,  0,  0, 20, 20,
		-10,-20,-20,-20,-20,-20,-20,-10,
		-20,-30,-30,-40,-40,-30,-30,-20,
		-30,-40,-40,-50,-50,-40,-40,-30,
		-30,-40,-40,-50,-50,-40,-40,-30,
		-30,-40,-40,-50,-50,-40,-40,-30,
		-30,-40,-40,-50,-50,-40,-40,-30
		};

		egKingTable[0] = {
		-50,-30,-30,-30,-30,-30,-30,-50
		-30,-30,  0,  0,  0,  0,-30,-30,
		-30,-10, 20, 30, 30, 20,-10,-30,
		-30,-10, 30, 40, 40, 30,-10,-30,
		-30,-10, 30, 40, 40, 30,-10,-30,
		-30,-10, 20, 30, 30, 20,-10,-30,
		-30,-20,-10,  0,  0,-10,-20,-30,
		-50,-40,-30,-20,-20,-30,-40,-50,
		};

		for (Square sq = 0; sq < 64; ++sq) {
			Square rel = sq ^ 56;
			pawnTable[1][rel] = pawnTable[0][sq];
			knightTable[1][rel] = knightTable[0][sq];
			bishopTable[1][rel] = bishopTable[0][sq];
			rookTable[1][rel] = rookTable[0][sq];
			queenTable[1][rel] = queenTable[0][sq];
			mgKingTable[1][rel] = mgKingTable[0][sq];
			egKingTable[1][rel] = egKingTable[0][sq];
		}
	}
}

inline Score evaluate(Board& board) {
	using namespace eval;

	Color us = board.sideToMove;
	Color them = !us;
	Score result = 
		getPieceValue(PAWN) * (board.pieces(us, PAWN).popcount() - board.pieces(them, PAWN).popcount()) +
		getPieceValue(KNIGHT) * (board.pieces(us, KNIGHT).popcount() - board.pieces(them, KNIGHT).popcount()) +
		getPieceValue(BISHOP) * (board.pieces(us, BISHOP).popcount() - board.pieces(them, BISHOP).popcount()) +
		getPieceValue(ROOK) * (board.pieces(us, ROOK).popcount() - board.pieces(them, ROOK).popcount()) +
		getPieceValue(QUEEN) * (board.pieces(us, QUEEN).popcount() - board.pieces(them, QUEEN).popcount());

	bool isEndgame = !board.pieces(QUEEN).popcount();
	Score pieceSq = 0;
	for (Square sq = A1; sq < N_SQUARES; ++sq) {
		Piece pc = board.getPiece(sq);
		PieceType pt = bb::getPieceType(pc);
		Color c = bb::getColor(pt);

		switch (pt) {
		case PAWN:
			pieceSq += pawnTable[c][sq];
			break;
		case KNIGHT:
			pieceSq += knightTable[c][sq];
			break;
		case BISHOP:
			pieceSq += bishopTable[c][sq];
			break;
		case ROOK:
			pieceSq += rookTable[c][sq];
			break;
		case QUEEN:
			pieceSq += queenTable[c][sq];
			break;
		case KING:
			if (!isEndgame)
				pieceSq += mgKingTable[c][sq];
			else
				pieceSq += egKingTable[c][sq];
			break;
		}
	}
	result = us == WHITE ? result + pieceSq : result - pieceSq;

	return result;
}