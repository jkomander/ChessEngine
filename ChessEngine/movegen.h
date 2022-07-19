#pragma once

#include<iostream>

#include"bitboard.h"
#include"board.h"

using MoveList = std::vector<Move>;

template<Color c>
void generatePawnMoves(Board& board, MoveList& moveList);
template<Color c, PieceType pt>
void generatePieceMoves(Board& board, MoveList& moveList);
template<Color c>
void generateKingMoves(Board& board, MoveList& moveList);
MoveList generateMoves(Board& board);
bool isLegal(Move move, Board& board);
bool isUnderAttack(Color us, Square sq, Board& board);
bool isInCheck(Board& board);

template<bool Root=true>
inline uint64_t perft(Board& board, int depth) {
	uint64_t cnt, nodes = 0;
	const bool leaf = (depth == 2);

	for (auto& move : generateMoves(board)) {
		if (Root && depth <= 1)
			cnt = 1, ++nodes;
		else {
			board.applyMove(move);
			cnt = leaf ? generateMoves(board).size() : perft<false>(board, depth - 1);
			nodes += cnt;
			board.undoMove();
		}
		if (Root)
			std::cout << move.toString() << ": " << cnt << "\n";
	}
	return nodes;
}

inline void append(MoveList& a, MoveList& b) {
	a.insert(a.end(), b.begin(), b.end());
}

inline void print(MoveList& moveList) {
	for (auto& m : moveList)
		std::cout << m.toString() << " ";
	std::cout << "\n";
}