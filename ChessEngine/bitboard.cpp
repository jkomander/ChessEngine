#include<iostream>

#include"bitboard.h"

ostream& operator<<(ostream& os, Bitboard b) {
	for (Rank r = RANK_8; r >= RANK_1; --r) {
		for (File f = FILE_A; f <= FILE_H; ++f) {
			Square sq = bb::getSquare(f, r);
			os << (b.at(sq) ? 'X' : '.') << ' ';
		}
		os << '\n';
	}
	return os;
}

string Move::toString() {
	string s = bb::toString(from) + bb::toString(to);

	switch (promotion) {
	case Promotion::None:
		return s;
	case Promotion::Knight:
		return s + 'n';
	case Promotion::Bishop:
		return s + 'b';
	case Promotion::Rook:
		return s + 'r';
	case Promotion::Queen:
		return s + 'q';
	};
}

PieceType Move::getPieceType(uint8_t promotion) {
	switch (promotion) {
	case None:
		return NO_PIECE_TYPE;
	case Knight:
		return KNIGHT;
	case Bishop:
		return BISHOP;
	case Rook:
		return ROOK;
	case Queen:
		return QUEEN;
	}
}