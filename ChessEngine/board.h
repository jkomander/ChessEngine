#pragma once

#include<array>
#include<vector>

#include"bitboard.h"
#include"random.h"

const std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

enum CastlingRights {
	NO_CASTLING,
	WHITE_KING_SIDE,
	WHITE_QUEEN_SIDE = 1 << 1,
	BLACK_KING_SIDE = 1 << 2,
	BLACK_QUEEN_SIDE = 1 << 3,

	KING_SIDE = WHITE_KING_SIDE | BLACK_KING_SIDE,
	QUEEN_SIDE = WHITE_QUEEN_SIDE | BLACK_QUEEN_SIDE,
	WHITE_CASTLING = WHITE_KING_SIDE | WHITE_QUEEN_SIDE,
	BLACK_CASTLING = BLACK_KING_SIDE | BLACK_QUEEN_SIDE,
	ANY_CASTLING = WHITE_CASTLING | BLACK_CASTLING,
};

const Bitboard WHITE_KING_SIDE_PATH = Bitboard(0x60);
const Bitboard WHITE_QUEEN_SIDE_PATH = Bitboard(0xe);
const Bitboard BLACK_KING_SIDE_PATH = Bitboard(0x60).mirrored();
const Bitboard BLACK_QUEEN_SIDE_PATH = Bitboard(0xe).mirrored();

struct Castlings {
	int data;

	void set(int cr) { data |= cr; }
	void reset(int cr) { data &= ~cr; }
	bool canCastle(int cr) { return data & cr; }
	bool noCastling() { return data == 0; }
};

struct KingAttackInfo {
	Bitboard pinned;
	Bitboard attacked;
	bool doubleCheck;
	bool upToDate;

	bool check() { return bool(attacked); }
};

struct BoardStatus {
	int plyCount;
	int fiftyMoveCount;
	Castlings castlings;
	Key zobrist;

	int repetitions;
	Square epSquare;
	Piece captured;
	Move move;
	KingAttackInfo kingAttackInfo;

	friend std::ostream& operator<<(std::ostream& os, BoardStatus& bs);
};

struct Board {
	std::array<Piece, N_SQUARES>board;
	std::array<Bitboard, N_PIECE_TYPES>pieceBB;
	std::array<Bitboard, N_COLORS>colorBB;
	Bitboard occupiedBB;
	Color sideToMove;
	std::vector<BoardStatus>history;
	BoardStatus* st;

	Board() = default;
	Board(const std::string& FEN);

	friend std::ostream& operator<<(std::ostream& os, Board& board);

	std::string fen();

	Piece getPiece(Square sq);
	template<bool updateZobrist = true>
	void setPiece(Piece pc, Square sq);
	template<bool updateZobrist = true>
	void removePiece(Square sq);
	template<bool updateZobrist = true>
	void movePiece(Square from, Square to);

	BoardStatus* getBoardStatus();
	Key key();
	// King square
	Square ksq(Color c);

	bool canCastle(int cr);
	bool noCastling();

	void applyMove(Move move);
	void undoMove();
	void applyNullMove();
	void undoNullMove();

	Bitboard color(Color c);
	Bitboard pieces(PieceType pt);
	Bitboard pieces(Color c, PieceType pt);
	Bitboard occupied();

	// Draw by fifty-move rule or threefold repetition
	bool isDraw();
	
	bool isCapture(Move move);
	bool givesCheck(Move move);
	// tests whether a move from the transposition table or the principal variation is pseudo-legal
	bool isPseudoLegal(Move move);

	void generateKingAttackInfo(KingAttackInfo& k);
};

inline Piece Board::getPiece(Square sq) {
	return board[sq];
}

inline BoardStatus* Board::getBoardStatus() {
	return &history.back();
}

inline bool Board::canCastle(int cr) {
	return st->castlings.canCastle(cr);
}

inline bool Board::noCastling() {
	return st->castlings.noCastling();
}

inline Bitboard Board::color(Color c) {
	return colorBB[c];
}

inline Bitboard Board::pieces(PieceType pt) {
	return pieceBB[pt - 1];
}

inline Bitboard Board::pieces(Color c, PieceType pt) {
	return color(c) & pieces(pt);
}

inline Bitboard Board::occupied() {
	return occupiedBB;
}

inline Square Board::ksq(Color c) {
	return pieces(c, KING).LSB();
}

inline Key Board::key() {
	return st->zobrist;
}

inline bool Board::isDraw() {
	return st->repetitions >= 3 || st->fiftyMoveCount >= 2 * 50;
}

namespace Zobrist {
	inline std::array<std::array<Key, N_SQUARES>, 16>psq;
	inline Key side;
	inline std::array<Key, 16>castling;
	inline std::array<Key, N_FILES>enPassant;

	inline void init() {
		for (int i = 0; i < 16; ++i) {
			for (int j = 0; j < N_SQUARES; ++j)
				psq[i][j] = random::getULL();
		}
		side = random::getULL();
		for (auto& k : castling)
			k = random::getULL();
		for (auto& k : enPassant)
			k = random::getULL();
	}
}

inline bool Board::isCapture(Move move) {
	return getPiece(move.to) || move.isEnPassant();
}