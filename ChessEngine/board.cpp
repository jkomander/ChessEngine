#include<ostream>
#include<sstream>
#include<string>

#include"attacks.h"
#include"board.h"
#include"movegen.h"

Board::Board(const std::string& FEN) {
	for (auto& pc : board) {
		pc = NO_PIECE;
	}

	BoardStatus bs = BoardStatus();
	history = std::vector<BoardStatus>();
	history.push_back(bs);
	st = getBoardStatus();

	std::istringstream ss(FEN);
	std::string token;

	ss >> token;
	Square sq = A8;
	Piece pc;
	for (char c : token) {
		if ((pc = pieceToChar.find(c)) != std::string::npos) {
			setPiece<false>(pc, sq);
			sq += EAST;
		}

		else if (isdigit(c))
			sq += (c - '0') * EAST;
			
		else if (c == '/') {
			sq += 2 * SOUTH;
		}
	}

	ss >> token;
	sideToMove = (token == "w") ? WHITE : BLACK;

	ss >> token;
	for (char c : token) {
		switch (c) {
		case 'K':
			st->castlings.set(WHITE_KING_SIDE);
			break;
		case 'Q':
			st->castlings.set(WHITE_QUEEN_SIDE);
			break;
		case 'k':
			st->castlings.set(BLACK_KING_SIDE);
			break;
		case 'q':
			st->castlings.set(BLACK_QUEEN_SIDE);
			break;
		}
	}

	ss >> token;

	st->epSquare = (token == "-") ? NO_SQUARE : stoi(token);

	ss >> st->fiftyMoveCount;
	
	ss >> st->plyCount;
	st->plyCount = 2 * (st->plyCount - 1) + sideToMove;

	st->zobrist = 42;
}

std::ostream& operator<<(std::ostream& os, Board& board) {
	for (Rank r = RANK_8; r >= RANK_1; --r) {
		for (File f = FILE_A; f <= FILE_H; ++f) {
			Piece pc = board.getPiece(bb::getSquare(f, r));
			if (pc == NO_PIECE)
				std::cout << ". ";
			else
				std::cout << pieceToChar[pc] << " ";
		}
		os << '\n';
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, BoardStatus& bs) {
	os << "plyCount: " << bs.plyCount << '\n'
		<< "fityMoveCount: " << bs.fiftyMoveCount << '\n'
		<< "castlings: " << bs.castlings.data << '\n'
		<< "repetitions: " << bs.repetitions << '\n'
		<< "epSquare: " << bb::toString(bs.epSquare) << '\n'
		<< "captured: " << bs.captured << '\n'
		<< "move: " << bs.move.toString() << '\n';
	return os;
}

std::string Board::fen() {
	std::stringstream ss;

	for (Rank r = RANK_8; r >= RANK_1; --r) {
		int emptyCount = 0;
		for (File f = FILE_A; f <= FILE_H; ++f) {
			Piece pc = getPiece(bb::getSquare(f, r));
			if (pc) {
				if (emptyCount)
					ss << emptyCount;
				ss << pieceToChar[pc];
				emptyCount = 0;
			}
			else ++emptyCount;
		}
		if (emptyCount) ss << emptyCount;
		if (r != RANK_1) ss << '/';
	}

	ss << ' ' << ((sideToMove == WHITE) ? 'w' : 'b');

	ss << ' ';
	if (canCastle(WHITE_KING_SIDE)) ss << 'K';
	if (canCastle(WHITE_QUEEN_SIDE)) ss << 'Q';
	if (canCastle(BLACK_KING_SIDE)) ss << 'k';
	if (canCastle(BLACK_QUEEN_SIDE)) ss << 'q';
	if (noCastling()) ss << '-';

	ss << ' ' << ((st->epSquare == NO_SQUARE) ? "-" : bb::toString(st->epSquare));
	ss << ' ' << st->fiftyMoveCount;
	ss << ' ' << 1 + (st->plyCount - sideToMove) / 2;

	return ss.str();
}

template<bool updateZobrist>
void Board::setPiece(Piece pc, Square sq) {
	board[sq] = pc;
	pieceBB[bb::getPieceType(pc) - 1].set(sq);
	colorBB[bb::getColor(pc)].set(sq);
	occupiedBB.set(sq);
	
	if (updateZobrist)
		st->zobrist ^= Zobrist::psq[pc][sq];
}

template<bool updateZobrist>
void Board::removePiece(Square sq) {
	Piece pc = getPiece(sq);
	board[sq] = NO_PIECE;
	pieceBB[bb::getPieceType(pc) - 1].clear(sq);
	colorBB[bb::getColor(pc)].clear(sq);
	occupiedBB.clear(sq);

	if (updateZobrist)
		st->zobrist ^= Zobrist::psq[pc][sq];
}

template<bool updateZobrist>
void Board::movePiece(Square from, Square to) {
	Piece pc = getPiece(from);
	setPiece<updateZobrist>(pc, to);
	removePiece<updateZobrist>(from);
}

void Board::applyMove(Move move) {
	Square from = move.from;
	Square to = move.to;
	Piece pc = getPiece(from);
	Color us = sideToMove;
	Color them = !us;
	Direction push = bb::pawnPush(us);

	BoardStatus bs = BoardStatus();
	bs.plyCount = st->plyCount + 1;
	bs.fiftyMoveCount = st->fiftyMoveCount + 1;
	bs.castlings = st->castlings;
	bs.zobrist = st->zobrist;
	bs.captured = move.moveType == Move::EN_PASSANT ? bb::getPiece(them, PAWN) : getPiece(to);
	bs.move = move;

	// reset ep square
	if (bs.epSquare != NO_SQUARE) {
		st->zobrist ^= Zobrist::enPassant[bb::file(bs.epSquare)];
		bs.epSquare = NO_SQUARE;
	}

	if (bs.captured) {
		bs.fiftyMoveCount = 0;
		Square capsq = to;

		if (move.moveType == Move::EN_PASSANT)
			capsq -= push;

		removePiece(capsq);

		if (bs.captured == bb::getPiece(them, ROOK)) {
			if (to == bb::relativeSquare(us, A8)) {
				st->zobrist ^= Zobrist::castling[bs.castlings.data];
				bs.castlings.reset(us ? WHITE_QUEEN_SIDE : BLACK_QUEEN_SIDE);
				st->zobrist ^= Zobrist::castling[bs.castlings.data];
			}
			else if (to == bb::relativeSquare(us, H8)) {
				st->zobrist ^= Zobrist::castling[bs.castlings.data];
				bs.castlings.reset(us ? WHITE_KING_SIDE : BLACK_KING_SIDE);
				st->zobrist ^= Zobrist::castling[bs.castlings.data];
			}
		}
	}

	if (pc == bb::getPiece(us, PAWN)) {
		bs.fiftyMoveCount = 0;

		if (to - from == 2 * push) {
			bs.epSquare = to - push;
			st->zobrist ^= Zobrist::enPassant[bb::file(bs.epSquare)];
		}
	}
	
	else if (pc == bb::getPiece(us, KING)) {
		st->zobrist ^= Zobrist::castling[bs.castlings.data];
		bs.castlings.reset(us ? BLACK_CASTLING : WHITE_CASTLING);
		st->zobrist ^= Zobrist::castling[bs.castlings.data];

		if (move.moveType == Move::CASTLING) {
			// move Rook
			if (to == bb::relativeSquare(us, C1))
				movePiece(bb::relativeSquare(us, A1), bb::relativeSquare(us, D1));
			else if (to == bb::relativeSquare(us, G1))
				movePiece(bb::relativeSquare(us, H1), bb::relativeSquare(us, F1));
		}
	}

	else if (pc == bb::getPiece(us, ROOK)) {
		if (from == bb::relativeSquare(us, A1)) {
			st->zobrist ^= Zobrist::castling[bs.castlings.data];
			bs.castlings.reset(us ? BLACK_QUEEN_SIDE : WHITE_QUEEN_SIDE);
			st->zobrist ^= Zobrist::castling[bs.castlings.data];
		}
		else if (from == bb::relativeSquare(us, H1)) {
			st->zobrist ^= Zobrist::castling[bs.castlings.data];
			bs.castlings.reset(us ? BLACK_KING_SIDE : WHITE_KING_SIDE);
			st->zobrist ^= Zobrist::castling[bs.castlings.data];
		}
	}

	if (move.moveType == Move::PROMOTION) {
		removePiece(from);
		pc = bb::getPiece(us, Move::getPieceType(move.promotion));
		setPiece(pc, to);
	}
	else movePiece(from, to);

	st->zobrist ^= Zobrist::side;

	sideToMove = !sideToMove;

	// compute repetitions
	bs.repetitions = 0;
	for (int i = (int)history.size() - 4; i > (int)history.size() - bs.fiftyMoveCount - 1; i -= 2) {
		if (i <= 0)
			break;
		if (history[i].zobrist == st->zobrist) {
			bs.repetitions = history[i].repetitions + 1;
		}
	}

	std::swap(st->zobrist, bs.zobrist);
	history.push_back(bs);
	st = getBoardStatus();
}

void Board::undoMove() {
	sideToMove = !sideToMove;

	Square from = st->move.from;
	Square to = st->move.to;
	Piece pc = getPiece(to);
	Color us = sideToMove;
	Color them = !us;
	Direction push = bb::pawnPush(us);

	if (st->move.moveType == Move::EN_PASSANT) {
		movePiece<false>(to, from);
		to -= push;
	}

	else if (st->move.moveType == Move::PROMOTION) {
		removePiece<false>(to);
		setPiece<false>(bb::getPiece(us, PAWN), from);
	}

	else if (st->move.moveType == Move::CASTLING) {
		bool isKingSide = to > from;
		Square rto = bb::relativeSquare(us, isKingSide ? F1 : D1);
		Square rfrom = bb::relativeSquare(us, isKingSide ? H1 : A1);
		movePiece<false>(rto, rfrom);
		movePiece<false>(to, from);
	}

	else movePiece<false>(to, from);

	if (st->captured) setPiece<false>(st->captured, to);

	history.pop_back();
	st = getBoardStatus();
}

void Board::applyNullMove() {
	BoardStatus bs;
	bs.plyCount = st->plyCount + 1;
	bs.fiftyMoveCount = st->fiftyMoveCount + 1;
	bs.castlings = st->castlings;
	bs.repetitions = 0;
	bs.epSquare = NO_SQUARE;
	bs.captured = NO_PIECE;
	bs.move = Move();
	history.push_back(bs);
	st = getBoardStatus();
	sideToMove = !sideToMove;
}

void Board::undoNullMove() {
	sideToMove = !sideToMove;
	history.pop_back();
	st = getBoardStatus();
}

bool Board::givesCheck(Move move) {
	Bitboard theirKingBB = pieces(!sideToMove, KING);
	Square ksq = theirKingBB.LSB();
	Piece pt = bb::getPieceType(getPiece(move.from));
	Bitboard occ = occupiedBB;
	
	// promotion
	if (move.isPromotion()) {
		occ.clear(move.from);
		pt = Move::getPieceType(move.promotion);
	}

	Bitboard attacked;
	switch (pt) {
	case PAWN: {
		attacked = attacks::pawnAttacks[sideToMove][move.to];
		break;
	}
	case KNIGHT: {
		attacked = attacks::knightAttacks[move.to];
		break;
	}
	case BISHOP: {
		attacked = attacks::attacks<BISHOP>(move.to, occ);
		break;
	}
	case ROOK: {
		attacked = attacks::attacks<ROOK>(move.to, occ);
		break;
	}
	case QUEEN: {
		attacked = attacks::attacks<QUEEN>(move.to, occ);
		break;
	}
	}
	if (attacked & theirKingBB)
		return true;


	// castling
	if (move.isCastling()) {
		occ.clear(move.from);
		Square rsq = sideToMove == WHITE ? move.to > move.from ? F1 : D1 : move.to > move.from ? F8 : D8;
		if (attacks::attacks<ROOK>(rsq, occ) & theirKingBB)
			return true;
	}

	// en passant
	else if (move.isEnPassant()) {
		occ.clear(move.from);
		occ.clear(move.to - bb::pawnPush(sideToMove));
		occ.set(move.to);
		if (isUnderAttack(!sideToMove, ksq, *this))
			return true;
	}

	// discovery
	else {
		occ.clear(move.from);
		occ.set(move.to);

		if (isUnderAttack(!sideToMove, ksq, *this))
			return true;
	}

	return false;
}