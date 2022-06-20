#include"eval.h"
#include"random.h"
#include"search.h"

Move Search::bestMove(Board& board) {
	time.stop = false;
	time.optimum = 7 * 24 * 3600 * 1000;
	time.start();
	Depth maxDepth = 256;

	Score alpha = -INFINITY_SCORE;
	Score beta = INFINITY_SCORE;
	Score delta;
	Score prevScore;
	Score score;
	Stack stack;
	Depth depth = 1;

	// iterative deepening
	for (;;) {
		if (depth > maxDepth)
			break;

		cout << "Depth: " << depth << "\n";

		if (depth < 4) {
			score = alphaBeta<Root>(board, &stack, alpha, beta, depth);
		}

		else {
			prevScore = score;
			delta = 25;
			alpha = prevScore - delta;
			beta = prevScore + delta;
			for (;;) {
				cout << "Bounds: " << alpha << " " << beta << "\n";
				score = alphaBeta<Root>(board, &stack, alpha, beta, depth);

				if (time.hitTimeLimit())
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

		if (time.hitTimeLimit())
			break;

		pv.clear();
		for (int i = 0; i < stack.pvSize; ++i)
			pv.push_back(stack.pv[i]);

		cout << "PV: ";
		print(pv);
		cout << "Score: " << score << "\n";
		cout << "tt usage: " << tt.usage() << "\n";
		cout << "Elapsed time: " << time.elapsed() * 1e-3 << "\n";
		cout << "Nodes searched: " << nodes << "\n";
		cout << "\n";

		++depth;
	}
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

	// transposition table lookup
	Key key = board.key();
	bool found;
	TTEntry* tte = tt.probe(key, found);
	NodeType nodeType = NONE_NODE;

	if (!pvNode && found && depth <= tte->depth) {
		if (tte->nodeType == PV_NODE ||
			tte->nodeType == CUT_NODE && tte->score >= beta ||
			tte->nodeType == ALL_NODE && tte->score <= alpha) {
			return tte->score;
		}
	}

	MoveList moves = generateMoves(board);
	int moveCount = 0;

	// shuffle the moves randomly
	std::shuffle(moves.begin(), moves.end(), random::rng);

	// PV-move
	int offset = 0;
	if (pvNode && depth > 1 && (int)pv.size() - depth + 1 >= 0) {
		for (int i = 0; i < moves.size(); ++i) {
			if (moves[i] == pv[pv.size() - depth + 1]) {
				std::swap(moves[0], moves[i]);
				offset = 1;
				break;
			}
		}
	}
	// hash move
	else if (tte->nodeType == PV_NODE || tte->nodeType == CUT_NODE) {
		for (int i = 0; i < moves.size(); ++i) {
			if (moves[i] == tte->move) {
				std::swap(moves[0], moves[i]);
				offset = 1;
				break;
			}
		}
	}

	// history heuristic
	std::stable_sort(moves.begin() + offset, moves.end(),
		[&](Move m1, Move m2) {
			return history[board.getPiece(m1.from)][m1.to] > history[board.getPiece(m2.from)][m2.to];
		}
	);

	Score bestScore = -INFINITY_SCORE;
	Score score;
	for (auto move : moves) {
		++moveCount;

		if (rootNode && depth >= 11)
			cout << move << "\n";

		board.applyMove(move);

		if (pvNode && moveCount == 1)
			score = -alphaBeta<PV>(board, &stack, -beta, -alpha, depth - 1);
		else {
			score = -alphaBeta<NonPV>(board, &stack, -(alpha + 1), -alpha, depth - 1);
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
			return score;
		}

		if (score > alpha) {
			alpha = score;
			nodeType = PV_NODE;

			if (rootNode && depth >= 11)
				cout << score << "\n";
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

	Score bestScore = evaluate(board);
	if (bestScore >= beta)
		return bestScore;
	if (bestScore > alpha)
		alpha = bestScore;

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
	std::mt19937_64 rng(0);
	std::shuffle(moves.begin(), moves.end(), rng);

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