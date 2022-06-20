#pragma once

#include"search.h"

enum NodeType {
	NONE_NODE, PV_NODE, CUT_NODE, ALL_NODE
};

struct TTEntry {
	Key key;
	Score score;
	Move move;
	Depth depth;
	NodeType nodeType;

	void save(Key key, Score score, Move move, Depth depth, NodeType nodeType);
};

struct TranspositionTable {
	std::unique_ptr<TTEntry[]>entries;
	uint64_t size;
	uint64_t mask;

	void setSize(uint64_t mb);
	void clear();
	TTEntry* get(Key key) {
		return &entries[key & mask];
	}
	TTEntry* probe(Key key, bool& found);
	double usage();
};

extern TranspositionTable tt;