#include"tt.h"

// global transposition table
TranspositionTable tt;

// input in mebibytes (1024^2 bytes)
void TranspositionTable::setSize(uint64_t mb) {
	uint64_t bytes = mb * 1024 * 1024;
	uint64_t maxSize = bytes / sizeof(TTEntry);

	size = 1;
	for (;;) {
		uint64_t newSize = 2 * size;
		if (newSize > maxSize)
			break;
		size = newSize;
	}

	mask = size - 1;
	entries = std::make_unique<TTEntry[]>(size);
	clear();
}

void TranspositionTable::clear() {
	std::memset(entries.get(), 0, size * sizeof(TTEntry));
}

TTEntry* TranspositionTable::probe(Key key, bool& found) {
	TTEntry* entry = get(key);
	found = key == entry->key;
	return entry;
}

double TranspositionTable::usage() {
	int cnt = 0;
	int n = 1000;
	for (int i = 1; i < n + 1; ++i) {
		if (entries[i].key)
			cnt += 1;
	}
	return (double)cnt / n;
}

void TTEntry::save(Key key, Score score, Move move, Depth depth, NodeType nodeType) {
	if (nodeType == PV_NODE ||
		key != this->key ||
		depth + 4 > this->depth) {
		this->key = key;
		this->score = score;
		this->move = move;
		this->depth = depth;
		this->nodeType = nodeType;
	}
}