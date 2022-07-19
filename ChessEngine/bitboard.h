#pragma once

#include<cassert>
#include<cstdint>
#include<intrin.h>
#include<string>

using Color = bool;
using Piece = int;
using PieceType = int;
using File = int;
using Rank = int;
using Direction = int;
using Square = uint8_t;

using Key = uint64_t;

using Score = int16_t;
using Depth = int;

enum Colors {
	WHITE,
	BLACK,
	N_COLORS
};

enum Pieces {
	NO_PIECE,
	WHITE_PAWN,
	WHITE_KNIGHT,
	WHITE_BISHOP,
	WHITE_ROOK,
	WHITE_QUEEN,
	WHITE_KING,
	BLACK_PAWN = WHITE_PAWN + 8,
	BLACK_KNIGHT,
	BLACK_BISHOP,
	BLACK_ROOK,
	BLACK_QUEEN,
	BLACK_KING,
	N_PIECES = 12
};

enum PieceTypes {
	NO_PIECE_TYPE,
	PAWN,
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	KING,
	N_PIECE_TYPES = 6
};

enum Squares {
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8,
	NO_SQUARE,
	N_SQUARES = 64
};

enum Files {
	FILE_A,
	FILE_B,
	FILE_C,
	FILE_D,
	FILE_E,
	FILE_F,
	FILE_G,
	FILE_H,
	N_FILES
};

enum Ranks {
	RANK_1,
	RANK_2,
	RANK_3,
	RANK_4,
	RANK_5,
	RANK_6,
	RANK_7,
	RANK_8,
	N_RANKS
};

enum Directions {
	NORTH = 8,
	EAST = 1,
	SOUTH = -NORTH,
	WEST = -EAST,
	NORTHEAST = NORTH + EAST,
	NORTHWEST = NORTH + WEST,
	SOUTHEAST = SOUTH + EAST,
	SOUTHWEST = SOUTH + WEST,
	N_DIRECTIONS = 8
};

const std::string pieceToChar = " PNBRQK  pnbrqk";

namespace bb {
	inline Piece getPiece(Color c, PieceType pt) {
		assert(pt != NO_PIECE_TYPE);
		return c * 8 + pt;
	}

	inline Color getColor(Piece pc) {
		return pc & 8;
	}

	inline PieceType getPieceType(Piece pc) {
		return pc & 7;
	}

	inline Piece relativePiece(Piece pc) {
		return pc ? pc ^ 8 : pc;
	}

	inline Square getSquare(File file, Rank rank) {
		return 8 * rank + file;
	}

	inline Square getSquare(const std::string& str) {
		return getSquare(File(str[0] - 'a'), Rank(str[1] - '1'));
	}

	inline File file(Square sq) { return sq % 8; }
	inline Rank rank(Square sq) { return sq / 8; }

	inline void mirror(Square& sq) { sq ^= A8; }

	constexpr inline Square relativeSquare(Color c, Square sq) { return c ? sq ^ A8 : sq; }

	inline std::string toString(Square sq) {
		return std::string(1, 'a' + file(sq)) + std::string(1, '1' + rank(sq));
	}

	inline bool isValid(Square sq) {
		return sq >= A1 && sq <= H8;
	}

	inline int distance(Square s1, Square s2) {
		return std::max(abs(file(s1) - file(s2)), abs(rank(s1) - rank(s2)));
	}

	inline Direction pawnPush(Color c) {
		return c ? SOUTH : NORTH;
	}
} // namespace bb

struct Bitboard {
	uint64_t data;

	Bitboard() :data(0) {}
	Bitboard(uint64_t data) :data(data) {}
	static Bitboard fromSquare(Square sq) { return { uint64_t(1) << sq }; }

	friend Bitboard operator&(const Bitboard& a, const Bitboard& b) {
		return { a.data & b.data };
	}

	friend Bitboard operator|(const Bitboard& a, const Bitboard& b) {
		return { a.data | b.data };
	}

	friend Bitboard operator^(const Bitboard& a, const Bitboard& b) {
		return { a.data ^ b.data };
	}

	friend Bitboard operator-(const Bitboard& a, const Bitboard& b) {
		return { a.data & ~b.data };
	}

	void operator&=(const Bitboard& b) {
		this->data &= b.data;
	}

	void operator|=(const Bitboard& b) {
		this->data |= b.data;
	}

	void operator^=(const Bitboard& b) {
		this->data ^= b.data;
	}

	void operator-=(const Bitboard& b) {
		this->data &= ~b.data;
	}

	Bitboard operator~() {
		return { ~this->data };
	}

	explicit operator bool() {
		return bool(this->data);
	}

	friend Bitboard operator<<(const Bitboard& b, int n) {
		return{ b.data << n };
	}

	friend Bitboard operator>>(const Bitboard& b, int n) {
		return{ b.data >> n };
	}

	bool operator==(const Bitboard& b) {
		return data == b.data;
	}

	friend std::ostream& operator<<(std::ostream& os, Bitboard b);

	bool at(Square sq) { return data & uint64_t(1) << sq; }
	void set(Square sq) { data |= uint64_t(1) << sq; }
	void clear(Square sq) { data &= ~(uint64_t(1) << sq); }

	void mirror() { data = _byteswap_uint64(data); }
	Bitboard mirrored() { return Bitboard(_byteswap_uint64(data));}

	int popcount() { return (int)__popcnt64(data); }

	template<Direction d>Bitboard shift();

	bool intersects(Bitboard& other) { return this->data & other.data; }

	Square LSB() {
		unsigned long idx;
		_BitScanForward64(&idx, data);
		return Square(idx);
	}

	Square popLSB() {
		Square sq = LSB();
		data &= data - 1;
		return sq;
	}

	Square MSB() {
		unsigned long idx;
		_BitScanReverse64(&idx, data);
		return Square(idx);
	}

	Square popMSB() {
		Square sq = MSB();
		data &= data - 1;
		return sq;
	}
};

const Bitboard FILE_A_BB = Bitboard(0x101010101010101);
const Bitboard FILE_B_BB = FILE_A_BB << 1;
const Bitboard FILE_C_BB = FILE_A_BB << 2;
const Bitboard FILE_D_BB = FILE_A_BB << 3;
const Bitboard FILE_E_BB = FILE_A_BB << 4;
const Bitboard FILE_F_BB = FILE_A_BB << 5;
const Bitboard FILE_G_BB = FILE_A_BB << 6;
const Bitboard FILE_H_BB = FILE_A_BB << 7;

const Bitboard RANK_1_BB = Bitboard(0xff);
const Bitboard RANK_2_BB = RANK_1_BB << 8;
const Bitboard RANK_3_BB = RANK_1_BB <<	16;
const Bitboard RANK_4_BB = RANK_1_BB << 24;
const Bitboard RANK_5_BB = RANK_1_BB << 32;
const Bitboard RANK_6_BB = RANK_1_BB << 40;
const Bitboard RANK_7_BB = RANK_1_BB << 48;
const Bitboard RANK_8_BB = RANK_1_BB << 56;

struct Move {
	enum Promotion :uint8_t {
		None, Knight, Bishop, Rook, Queen
	};

	enum MoveType :uint8_t {
		NORMAL, EN_PASSANT, PROMOTION, CASTLING
	};

	Square from;
	Square to;
	uint8_t promotion;
	uint8_t moveType;

	Move() = default;
	Move(Square from, Square to) :
		Move(from, to, Promotion::None, MoveType::NORMAL) {}
	Move(Square from, Square to, MoveType moveType) :
		Move(from, to, Promotion::None, moveType) {}
	Move(Square from, Square to, uint8_t promotion, uint8_t moveType) :
		from(from), to(to), promotion(promotion), moveType(moveType) {}

	bool operator==(const Move& other) {
		return this->from == other.from &&
			this->to == other.to &&
			this->promotion == other.promotion &&
			this->moveType == other.moveType;
	}

	bool operator!=(const Move& other) {
		return !(*this == other);
	}

	explicit operator bool() {
		return *this != Move();
	}

	friend std::ostream& operator<<(std::ostream& os, Move& move) {
		os << move.toString();
		return os;
	}

	std::string toString();

	void mirror() {
		bb::mirror(from);
		bb::mirror(to);
	}

	static PieceType getPieceType(uint8_t promotion);

	inline bool isNormal() {
		return moveType == Move::NORMAL;
	}

	inline bool isEnPassant() {
		return moveType == Move::EN_PASSANT;
	}

	inline bool isPromotion() {
		return moveType == Move::PROMOTION;
	}

	inline bool isCastling() {
		return moveType == Move::CASTLING;
	}
};

template<Direction d>
Bitboard Bitboard::shift() {
	Bitboard b = *this;
	if (d == NORTH) return b << NORTH;
	else if (d == SOUTH) return b >> NORTH;
	else if (d == 2 * NORTH) return b << 2 * NORTH;
	else if (d == 2 * SOUTH) return b >> 2 * NORTH;
	else if (d == NORTHEAST) return (b << NORTHEAST) - FILE_A_BB;
	else if (d == NORTHWEST) return (b << NORTHWEST) - FILE_H_BB;
	else if (d == SOUTHEAST) return (b >> NORTHWEST) - FILE_A_BB;
	else if (d == SOUTHWEST) return (b >> NORTHEAST) - FILE_H_BB;
	else return Bitboard();
}