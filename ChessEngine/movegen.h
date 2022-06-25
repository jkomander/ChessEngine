#pragma once

#include<iostream>

#include"bitboard.h"
#include"board.h"

using MoveList = std::vector<Move>;

struct KingAttackInfo {
    Bitboard pinned;
    Bitboard attacked;
    bool doubleCheck;

    KingAttackInfo(Board& board);

    bool check() { return bool(attacked); }
};

template<Color c>
void generatePawnMoves(Board& board, MoveList& moveList);
template<Color c, PieceType pt>
void generatePieceMoves(Board& board, MoveList& moveList);
template<Color c>
void generateKingMoves(Board& board, MoveList& moveList);
MoveList generateMoves(Board& board);
bool isLegal(Move move, Board& board, KingAttackInfo& kingAttackInfo);
bool isUnderAttack(Color us, Square sq, Board& board);
bool isInCheck(Board& board);

template<bool Root>
uint64_t perft(Board& board, int depth);

inline void append(MoveList& a, MoveList& b) {
	a.insert(a.end(), b.begin(), b.end());
}

inline void print(MoveList& moveList) {
	for (auto& m : moveList)
		std::cout << m.toString() << " ";
	std::cout << "\n";
}