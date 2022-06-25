#include"attacks.h"
#include"movegen.h"

const std::array<Move::Promotion, 4>promotions =
{ Move::Knight, Move::Bishop, Move::Rook, Move::Queen };

KingAttackInfo::KingAttackInfo(Board& board) {
	pinned = Bitboard();
	attacked = Bitboard();

	Color us = board.sideToMove;
	Color them = !us;
	Square ksq = board.ksq(us);
	Bitboard ourTeam = board.color(us);
	Bitboard theirTeam = board.color(them);
	Bitboard queens = board.pieces(them, QUEEN);
	Bitboard bishops = board.pieces(them, BISHOP) | queens;
	Bitboard rooks = board.pieces(them, ROOK) | queens;

	int attackerCount = 0;

	Bitboard pawnAttacks = attacks::pawnAttacks[us][ksq] & board.pieces(them, PAWN);
	if (pawnAttacks) {
		attacked |= pawnAttacks;
		++attackerCount;
	}

	Bitboard knightAttacks = attacks::knightAttacks[ksq] & board.pieces(them, KNIGHT);
	if (knightAttacks) {
		attacked |= knightAttacks;
		++attackerCount;
	}

	if (attacks::bishopAttacks[ksq].intersects(bishops)) {
		for (auto direction : attacks::bishopDirections) {
			Square sq = ksq;
			Square pinnedSq;
			bool found = false;
			Bitboard attackLine = Bitboard();
			for (;;) {
				Square sq_ = sq;
				sq += direction;
				if (!bb::isValid(sq) || bb::distance(sq, sq_) != 1)
					break;
				if (ourTeam.at(sq)) {
					if (!found) {
						found = true;
						pinnedSq = sq;
					}
					else break;
				}
				if (!found) attackLine.set(sq);
				if (theirTeam.at(sq)) {
					if (bishops.at(sq)) {
						if (found)
							pinned.set(pinnedSq);
						else {
							attacked |= attackLine;
							++attackerCount;
						}
					}
					break;
				}
			}
		}
	}

	if (attacks::rookAttacks[ksq].intersects(rooks)) {
		for (auto direction : attacks::rookDirections) {
			Square sq = ksq;
			Square pinnedSq;
			bool found = false;
			Bitboard attackLine = Bitboard();
			for (;;) {
				Square sq_ = sq;
				sq += direction;
				if (!bb::isValid(sq) || bb::distance(sq, sq_) != 1)
					break;
				if (ourTeam.at(sq)) {
					if (!found) {
						found = true;
						pinnedSq = sq;
					}
					else break;
				}
				if (!found) attackLine.set(sq);
				if (theirTeam.at(sq)) {
					if (rooks.at(sq)) {
						if (found) pinned.set(pinnedSq);
						else {
							attacked |= attackLine;
							++attackerCount;
						}
					}
					break;
				}
			}
		}
	}
	doubleCheck = attackerCount == 2;
}

template<Color c>
void generatePawnMoves(Board& board, MoveList& moveList) {
	constexpr Color us = c;
	constexpr Color them = !c;

	constexpr Direction up = c == WHITE ? NORTH : SOUTH;
	constexpr Direction upRight = c == WHITE ? NORTHEAST : SOUTHEAST;
	constexpr Direction upLeft = c == WHITE ? NORTHWEST : SOUTHWEST;

	Bitboard relative_rank_8_bb = c == WHITE ? RANK_8_BB : RANK_1_BB;
	Bitboard relative_rank_4_bb = c == WHITE ? RANK_4_BB : RANK_5_BB;

	Bitboard empty = ~board.occupied();
	Bitboard theirTeam = board.color(them);
	Bitboard ourPawns = board.pieces(us, PAWN);

	Bitboard attacks;
	Square to;
	Square from;

	Bitboard singlePawnPushTargets = ourPawns.shift<up>() & empty;
	Bitboard upRightBB = ourPawns.shift<upRight>();
	Bitboard upLeftBB = ourPawns.shift<upLeft>();
	Bitboard upRightCaptures = upRightBB & theirTeam;
	Bitboard upLeftCaptures = upLeftBB & theirTeam;

	// single pawn pushes
	attacks = singlePawnPushTargets - relative_rank_8_bb;
	while (attacks) {
		to = attacks.popLSB();
		from = to - up;
		moveList.emplace_back(from, to);
	}
	// pawn push promotions
	attacks = singlePawnPushTargets & relative_rank_8_bb;
	while (attacks) {
		to = attacks.popLSB();
		from = to - up;
		for (auto promotion : promotions)
			moveList.emplace_back(from, to, promotion, Move::PROMOTION);
	}
	// double pawn pushes
	attacks = singlePawnPushTargets.shift<up>() &
		empty &
		relative_rank_4_bb;
	while (attacks) {
		to = attacks.popLSB();
		from = to - 2 * up;
		moveList.emplace_back(from, to);
	}
	// pawn captures
	attacks = upRightCaptures - relative_rank_8_bb;

	while (attacks) {
		to = attacks.popLSB();
		from = to - upRight;
		moveList.emplace_back(from, to);
	}
	attacks = upLeftCaptures - relative_rank_8_bb;
	while (attacks) {
		to = attacks.popLSB();
		from = to - upLeft;
		moveList.emplace_back(from, to);
	}
	// pawn capture promotions
	attacks = upRightCaptures & relative_rank_8_bb;
	while (attacks) {
		to = attacks.popLSB();
		from = to - upRight;
		for (auto promotion : promotions)
			moveList.emplace_back(from, to, promotion, Move::PROMOTION);
	}
	attacks = upLeftCaptures & relative_rank_8_bb;
	while (attacks) {
		to = attacks.popLSB();
		from = to - upLeft;
		for (auto promotion : promotions)
			moveList.emplace_back(from, to, promotion, Move::PROMOTION);
	}
	// en passant
	if (board.st->epSquare != NO_SQUARE) {
		attacks = upRightBB & Bitboard::fromSquare(board.st->epSquare);
		if (attacks) {
			to = board.st->epSquare;
			from = to - upRight;
			moveList.emplace_back(from, to, Move::EN_PASSANT);
		}
		attacks = upLeftBB & Bitboard::fromSquare(board.st->epSquare);
		if (attacks) {
			to = board.st->epSquare;
			from = to - upLeft;
			moveList.emplace_back(from, to, Move::EN_PASSANT);
		}
	}
}

template<Color c, PieceType pt>
void generatePieceMoves(Board& board, MoveList& moveList) {
	Bitboard friendly = board.color(c);
	Bitboard pieces = board.pieces(c, pt);

	while (pieces) {
		Square from = pieces.popLSB();
		Bitboard attacks = attacks::attacks<pt>(from, board.occupied()) - friendly;

		while (attacks)
			moveList.emplace_back(from, attacks.popLSB());
	}
}

template<Color c>
void generateKingMoves(Board& board, MoveList& moveList) {
	Square ksq = board.ksq(c);
	Bitboard attacks = attacks::kingAttacks[ksq] - board.color(c);
	while (attacks)
		moveList.emplace_back(ksq, attacks.popLSB());

	// castling
	if (ksq == bb::relativeSquare(c, E1)) {
		Bitboard empty = ~board.occupied();
		Bitboard path1 = c == WHITE ? Bitboard(0xe) : Bitboard(0xe).mirrored();
		Bitboard path2 = c == WHITE ? Bitboard(0x60) : Bitboard(0x60).mirrored();

		if (board.canCastle(c == WHITE ? WHITE_QUEEN_SIDE : BLACK_QUEEN_SIDE) && (empty & path1) == path1)
			moveList.emplace_back(ksq, bb::relativeSquare(c, C1), Move::CASTLING);
		if (board.canCastle(c == WHITE ? WHITE_KING_SIDE : BLACK_KING_SIDE) && (empty & path2) == path2)
			moveList.emplace_back(ksq, bb::relativeSquare(c, G1), Move::CASTLING);
	}
}

MoveList generateMoves(Board& board) {
	MoveList moveList;
	KingAttackInfo kingAttackInfo(board);

	if (board.sideToMove == WHITE) {
		if (!kingAttackInfo.doubleCheck) {
			generatePawnMoves<WHITE>(board, moveList);
			generatePieceMoves<WHITE, KNIGHT>(board, moveList);
			generatePieceMoves<WHITE, BISHOP>(board, moveList);
			generatePieceMoves<WHITE, ROOK>(board, moveList);
			generatePieceMoves<WHITE, QUEEN>(board, moveList);
		}
		generateKingMoves<WHITE>(board, moveList);
	}

	else {
		if (!kingAttackInfo.doubleCheck) {
			generatePawnMoves<BLACK>(board, moveList);
			generatePieceMoves<BLACK, KNIGHT>(board, moveList);
			generatePieceMoves<BLACK, BISHOP>(board, moveList);
			generatePieceMoves<BLACK, ROOK>(board, moveList);
			generatePieceMoves<BLACK, QUEEN>(board, moveList);
		}
		generateKingMoves<BLACK>(board, moveList);
	}
	
	moveList.erase(
		remove_if(moveList.begin(), moveList.end(),
			[&](Move m) { return !isLegal(m, board, kingAttackInfo); }),
		moveList.end());
	return moveList;
}

bool isLegal(Move move, Board& board, KingAttackInfo& kingAttackInfo) {
	Square from = move.from;
	Square to = move.to;
	Piece pc = board.getPiece(from);
	Color us = board.sideToMove;
	Color them = !us;

	if (pc == bb::getPiece(us, KING)) {
		if (move.moveType == Move::CASTLING) {
			if (kingAttackInfo.check() ||
				to == bb::relativeSquare(us, C1) && 
				   (isUnderAttack(us, bb::relativeSquare(us, D1), board) ||
					isUnderAttack(us, bb::relativeSquare(us, C1), board)) ||
				to == bb::relativeSquare(us, G1) && 
				   (isUnderAttack(us, bb::relativeSquare(us, F1), board) ||
					isUnderAttack(us, bb::relativeSquare(us, G1), board)))
				return false;
		}
		else {
			board.applyMove(move);
			bool illegal = isUnderAttack(us, to, board);
			board.undoMove();
			if (illegal) return false;
		}
	}

	else if (kingAttackInfo.check() && (!kingAttackInfo.attacked.at(to) || kingAttackInfo.pinned.at(from)))
		return false;

	else {
		// pins when not in check
		if (kingAttackInfo.pinned.at(from)) {
			Square ksq = board.ksq(us);
			int dx_from = bb::file(from) - bb::file(ksq);
			int dy_from = bb::rank(from) - bb::rank(ksq);
			int dx_to = bb::file(to) - bb::file(ksq);
			int dy_to = bb::rank(to) - bb::rank(ksq);

			// north, south
			if (dx_from == 0 || dx_to == 0) {
				if (dx_from != dx_to)
					return false;
			}
			// east, west
			else if (dy_from == 0 || dy_to == 0) {
				if (dy_from != dy_to)
					return false;
			}
			// northeast, southeast, southwest, northwest
			else if (dx_from * dy_to != dy_from * dx_to)
				return false;
		}

		// en passant
		if (move.moveType == Move::EN_PASSANT) {
			board.applyMove(move);
			bool illegal = isUnderAttack(us, board.ksq(us), board);
			board.undoMove();
			if (illegal) return false;
		}
	}
	return true;
}

bool isUnderAttack(Color us, Square sq, Board& board) {
	Color them = !us;
	Bitboard occupied = board.occupied();
	return (attacks::pawnAttacks[us][sq] & board.pieces(them, PAWN) ||
		attacks::knightAttacks[sq] & board.pieces(them, KNIGHT) ||
		attacks::attacks<BISHOP>(sq, occupied) & (board.pieces(them, BISHOP) | board.pieces(them, QUEEN)) ||
		attacks::attacks<ROOK>(sq, occupied) & (board.pieces(them, ROOK) | board.pieces(them, QUEEN))) ||
		attacks::kingAttacks[sq] & board.pieces(them, KING);
}

bool isInCheck(Board& board) {
	return isUnderAttack(board.sideToMove, board.ksq(board.sideToMove), board);
}

template<bool Root>
uint64_t perft(Board& board, int depth) {
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