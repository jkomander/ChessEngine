#include"eval.h" 
#include"movepick.h"
#include"random.h"
#include"search.h"

Move Search::bestMove(Board& board) {
	time.start();
	
	Color us = board.sideToMove;
	time.maximum = time.time[us];
	if (time.maximum) {
		TimePoint delta = 100;
		time.optimum = (time.maximum < time.inc[us] ? time.maximum :
						0.1 * (time.maximum - time.inc[us]) + time.inc[us]) - delta;
	}
	else
		time.optimum = 7 * 24 * 3600 * 1000;
	std::cout << "Optimal time: " << 1e-3 * time.optimum << std::endl << "\n";

	nodes = 0;

	Depth maxDepth = 256;
	Score alpha = -INFINITY_SCORE;
	Score beta = INFINITY_SCORE;
	Score delta;
	Score prevScore;
	Score score;
	Stack stack;
	stack.ply = 0;
	Depth depth = 1;

	// iterative deepening
	for (;;) {
		if (depth > maxDepth)
			break;

		std::cout << "Depth: " << depth << std::endl;

		if (depth < 4) {
			score = alphaBeta<Root>(board, &stack, alpha, beta, depth);
		}

		else {
			prevScore = score;
			delta = 25;
			alpha = prevScore - delta;
			beta = prevScore + delta;
			for (;;) {
				std::cout << "Bounds: " << alpha << " " << beta << std::endl;
				score = alphaBeta<Root>(board, &stack, alpha, beta, depth);

				if (time.stop)
					break;

				if (score <= alpha) {
					beta = (alpha + beta) / 2;
					alpha -= delta;
				}
				else if (score >= beta) {
					beta += delta;
				}
				else
					break;

				delta += delta / 4;
			}
		}

		if (time.stop)
			break;

		pv.clear();
		for (int i = 0; i < stack.pvSize; ++i)
			pv.push_back(stack.pv[i]);

		std::cout << "PV: ";
		print(pv);
		std::cout << "Score: " << score << "\n";
		std::cout << "tt usage: " << tt.usage() << "\n";
		std::cout << "Elapsed time: " << time.elapsed() * 1e-3 << "\n";
		std::cout << "Nodes searched: " << nodes << "\n";
		std::cout << std::endl;

		++depth;
	}
	std::cout << "Elapsed time: " << time.elapsed() * 1e-3 << std::endl;
	return pv[0];
}

template<SearchType searchType>
Score Search::alphaBeta(Board& board, Stack* ss, Score alpha, Score beta, Depth depth) {
	constexpr bool rootNode = searchType == Root;
	constexpr bool pvNode = searchType != NonPV;

	++nodes;

	if (depth >= 5 && time.hitTimeLimit())
		time.stop = true;

	if (!rootNode && (board.isDraw() || time.stop))
		return DRAW_SCORE;

	if (depth == 0)
		return quiescence<pvNode ? PV : NonPV>(board, alpha, beta);

	Stack stack = Stack();
	stack.ply = ss->ply + 1;

	// transposition table lookup
	Key key = board.key();
	bool found;
	TTEntry* tte = tt.probe(key, found);
	NodeType nodeType = NONE_NODE;
	Move hashMove = Move();

	if (!pvNode && found && depth <= tte->depth) {
		if (tte->nodeType == PV_NODE ||
			tte->nodeType == CUT_NODE && tte->score >= beta ||
			tte->nodeType == ALL_NODE && tte->score <= alpha) {
			return tte->score;
		}
	}

	// PV-move
	if (pvNode && depth > 1 && (int)pv.size() - depth + 1 >= 0)
		hashMove = pv[pv.size() - depth + 1];

	// hash move
	else if (found && (tte->nodeType == PV_NODE || tte->nodeType == CUT_NODE))
		hashMove = tte->move;

	if (hashMove) {
		if (!board.isPseudoLegal(hashMove) || !isLegal(hashMove, board))
			hashMove = Move();
	}

	// first killer move
	Move killerMove1 = killer[ss->ply][0];
	if (!board.isPseudoLegal(killerMove1) || !isLegal(killerMove1, board))
		killerMove1 = Move();

	// second killer move
	Move killerMove2 = killer[ss->ply][1];
	if (!board.isPseudoLegal(killerMove2) || !isLegal(killerMove2, board))
		killerMove2 = Move();
	
	int moveCount = 0;
	MovePicker mp = MovePicker(this, &board, hashMove, killerMove1, killerMove2);
	Score bestScore = -INFINITY_SCORE;
	Score score;

	for (;;) {
		Move move = mp.next();

		if (!move)
			break;

		assert(board.isPseudoLegal(move)); 
		assert(isLegal(move, board));

		++moveCount;
		
		if (rootNode && depth >= 11)
			std::cout << move << std::endl;

		board.applyMove(move);

		if (pvNode && moveCount == 1)
			score = -alphaBeta<PV>(board, &stack, -beta, -alpha, depth - 1);
		else {
			score = -alphaBeta<NonPV>(board, &stack, -alpha - 1, -alpha, depth - 1);
			if (score > alpha && score < beta) {
				score = -alphaBeta<PV>(board, &stack, -beta, -alpha, depth - 1);
			}
		}

		board.undoMove();

		if (time.stop)
			return DRAW_SCORE;

		if (score > bestScore) {
			bestScore = score;

			if (pvNode) {
				ss->pv[0] = move;
				std::memcpy(ss->pv + 1, &stack, stack.pvSize * sizeof(Move));
				ss->pvSize = stack.pvSize + 1;
			}
		}

		if (score >= beta) {
			tte->save(key, score, move, depth, CUT_NODE);

			Piece pc = board.getPiece(move.from);
			history[pc][move.to] += depth * depth;

			if (move != killer[ss->ply][0]) {
				killer[ss->ply][1] = killer[ss->ply][0];
				killer[ss->ply][0] = move;
			}

			return score;
		}

		if (score > alpha) {
			alpha = score;
			nodeType = PV_NODE;

			if (rootNode && depth >= 11)
				std::cout << score << std::endl;
		}
	}

	// checkmate or stalemate
	if (!moveCount)
		return isInCheck(board) ? -MATE_SCORE : DRAW_SCORE;

	if (nodeType == PV_NODE)
		tte->save(key, bestScore, ss->pv[0], depth, PV_NODE);
	else
		tte->save(key, bestScore, Move(), depth, ALL_NODE);
		
	return bestScore;
}

template<SearchType searchType>
Score Search::quiescence(Board& board, Score alpha, Score beta) {
	constexpr bool pvNode = searchType == PV;

	++nodes;

	if (board.isDraw())
		return DRAW_SCORE;

	Score standPat = evaluate(board);
	Score bestScore = standPat;

	// standPat acts as a lower bound
	if (standPat >= beta)
		return standPat;
	if (standPat > alpha)
		alpha = standPat;

	MoveList moves = generateMoves(board);

	// checkmate or stalemate
	if (!moves.size())
		return isInCheck(board) ? -MATE_SCORE : DRAW_SCORE;

	// remove the non-captures
	moves.erase(
		remove_if(moves.begin(), moves.end(),
			[&](Move m) { return !(board.isCapture(m)); }),
		moves.end());

	// shuffle the moves randomly
	std::shuffle(moves.begin(), moves.end(), random::rng);

	Score score;
	for (auto move : moves) {
		board.applyMove(move);
		score = -quiescence<pvNode ? PV : NonPV>(board, -beta, -alpha);
		board.undoMove();

		if (score > bestScore) {
			bestScore = score;
			if (score >= beta)
				return score;
			if (score > alpha)
				alpha = score;
		}
	}
	return bestScore;
}