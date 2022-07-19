#pragma once

#include<array>

#include"bitboard.h"

namespace attacks {
	inline std::array<std::array<Bitboard, N_SQUARES>, N_COLORS>pawnAttacks;
	inline std::array<Bitboard, N_SQUARES>knightAttacks;
	inline std::array<Bitboard, N_SQUARES>kingAttacks;
	inline std::array<int, 8>knightDirections = { 17,10,-6,-15,-17,-10,6,15 };
	inline std::array<int, 8>kingDirections = { 8,9,1,-7,-8,-9,-1,7 };
	inline std::array<int, 4>bishopDirections = { 9,-7,-9,7 };
	inline std::array<int, 4>rookDirections = { 8,1,-8,-1 };

	inline std::array<Bitboard, 8>files = { FILE_A_BB, FILE_B_BB, FILE_C_BB, FILE_D_BB,
									   FILE_E_BB, FILE_F_BB, FILE_G_BB, FILE_H_BB };
	inline std::array<Bitboard, 8>ranks = { RANK_1_BB, RANK_2_BB, RANK_3_BB, RANK_4_BB,
									   RANK_5_BB, RANK_6_BB, RANK_7_BB, RANK_8_BB };
	inline std::array<Bitboard, 15>diagonals;
	inline std::array<Bitboard, 15>antiDiagonals;
	inline std::array<Bitboard, N_SQUARES>bishopAttacks;
	inline std::array<Bitboard, N_SQUARES>rookAttacks;

	enum Directions {
		NORTHEAST,
		NORTH,
		NORTHWEST,
		EAST,
		SOUTHEAST,
		SOUTH,
		SOUTHWEST,
		WEST,
		N_DIRECTIONS
	};
	inline std::array<std::array<Bitboard, N_SQUARES>, N_DIRECTIONS>rayAttacks;

	inline std::array<std::array<Bitboard, N_SQUARES>, N_SQUARES>inBetweenSquares;

	inline void init() {
		// knight attacks
		for (Square from = A1; from < N_SQUARES; ++from) {
			Bitboard attacks = Bitboard();
			for (const auto& d : knightDirections) {
				Square to = from + d;
				if (bb::isValid(to) && bb::distance(from, to) <= 2)
					attacks.set(to);
			}
			knightAttacks[from] = attacks;
		}

		// king attacks
		for (Square from = A1; from < N_SQUARES; ++from) {
			Bitboard attacks = Bitboard();
			for (const auto& d : kingDirections) {
				Square to = from + d;
				if (bb::isValid(to) && bb::distance(from, to) == 1)
					attacks.set(to);
			}
			kingAttacks[from] = attacks;
		}

		// diagonals
		diagonals[0] = Bitboard(0x8040201008040201);
		for (int i = 1; i < 8; ++i) {
			diagonals[i] = diagonals[0] << i;
			for (int j = 0; j < i - 1; ++j)
				diagonals[i] -= ranks[7 - j];
		}
		for (int i = 1; i < 8; ++i) {
			diagonals[i + 7] = diagonals[0] >> i;
			for (int j = 0; j < i - 1; ++j)
				diagonals[i + 7] -= ranks[j];
		}

		// anti-diagonals
		for (int i = 0; i < 15; ++i)
			antiDiagonals[i] = diagonals[i].mirrored();

		// bishop attacks on empty board
		for (Square sq = A1; sq < N_SQUARES; ++sq) {
			Bitboard attacks;
			for (auto& b : diagonals) {
				if (b.at(sq)) {
					attacks = b;
					break;
				}
			}
			for (auto& b : antiDiagonals) {
				if (b.at(sq)) {
					attacks ^= b;
					break;
				}
			}
			bishopAttacks[sq] = attacks;
		}

		// rook attacks on empty board
		for (Square sq = A1; sq < N_SQUARES; ++sq) {
			Bitboard attacks;
			for (auto& b : files) {
				if (b.at(sq)) {
					attacks = b;
					break;
				}
			}
			for (auto& b : ranks) {
				if (b.at(sq)) {
					attacks ^= b;
					break;
				}
			}
			rookAttacks[sq] = attacks;
		}

		// ray attacks
		for (Square sq = A1; sq < N_SQUARES; ++sq) {
			File file = bb::file(sq);
			Rank rank = bb::rank(sq);

			rayAttacks[NORTH][sq] = Bitboard(0x101010101010100) << sq;
			rayAttacks[SOUTH][sq] = Bitboard(0x80808080808080) >> 63 - sq;
			rayAttacks[EAST][sq] = Bitboard(0xfe) << sq;
			if (rank < RANK_8)
				rayAttacks[EAST][sq] -= ranks[rank + 1];
			rayAttacks[WEST][sq] = Bitboard(0x7f00000000000000) >> (sq ^ 7 ^ 56);
			if (rank > RANK_1)
				rayAttacks[WEST][sq] -= ranks[rank - 1];
			rayAttacks[NORTHEAST][sq] = Bitboard(0x8040201008040200) << sq;
			for (int i = 0; i < file - rank - 1; ++i)
				rayAttacks[NORTHEAST][sq] -= ranks[7 - i];
			rayAttacks[NORTHWEST][sq] = (Bitboard(0x102040810204000) >> bb::file(sq ^ 7)) << 8 * rank;
			for (int i = 7; i > file; --i)
				rayAttacks[NORTHWEST][sq] -= files[i];
			rayAttacks[SOUTHEAST][sq ^ 56] = rayAttacks[NORTHEAST][sq].mirrored();
			rayAttacks[SOUTHWEST][sq ^ 56] = rayAttacks[NORTHWEST][sq].mirrored();
		}

		// pawn attacks
		for (Square sq = A1; sq < N_SQUARES; ++sq) {
			Bitboard b = Bitboard::fromSquare(sq);
			pawnAttacks[WHITE][sq] |= b.shift<7>() | b.shift<9>();
			pawnAttacks[BLACK][sq] |= b.shift<-9>() | b.shift<-7>();
		}

		// squares in between two squares
		for (Square i = A1; i < N_SQUARES; ++i) {
			for (Square j = A1; j < N_SQUARES; ++j) {
				Bitboard b = Bitboard::fromSquare(j);
				for (const auto& arr : rayAttacks) {
					if (arr[i] & b) {
						inBetweenSquares[i][j] = arr[i] - arr[j] - b;
						break;
					}
				}
			}
		}
	}

	template<Directions d>
	inline Bitboard getRayAttacks(Square sq, const Bitboard& occupied) {
		Bitboard attacks = rayAttacks[d][sq];
		Bitboard blockers = attacks & occupied;
		if (blockers) {
			Square blocker = d <= EAST ? blockers.LSB() : blockers.MSB();
			attacks ^= rayAttacks[d][blocker];
		}
		return attacks;
	}

	template<PieceType pt>
	inline Bitboard attacks(Square sq, const Bitboard& occupied) {
		switch (pt) {
		case KNIGHT:
			return knightAttacks[sq];
		case BISHOP:
			return getRayAttacks<NORTHWEST>(sq, occupied) |
				   getRayAttacks<SOUTHWEST>(sq, occupied) |
				   getRayAttacks<SOUTHEAST>(sq, occupied) |
				   getRayAttacks<NORTHEAST>(sq, occupied);
		case ROOK:
			return getRayAttacks<NORTH>(sq, occupied) |
				   getRayAttacks<EAST>(sq, occupied) |
				   getRayAttacks<SOUTH>(sq, occupied) |
				   getRayAttacks<WEST>(sq, occupied);
		case QUEEN:
			return getRayAttacks<NORTHWEST>(sq, occupied) |
				   getRayAttacks<SOUTHWEST>(sq, occupied) |
				   getRayAttacks<SOUTHEAST>(sq, occupied) |
				   getRayAttacks<NORTHEAST>(sq, occupied) |
				   getRayAttacks<NORTH>(sq, occupied) |
				   getRayAttacks<EAST>(sq, occupied) |
				   getRayAttacks<SOUTH>(sq, occupied) |
				   getRayAttacks<WEST>(sq, occupied);
		};
	}
} // namespace attacks