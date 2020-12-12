#ifndef TTENTRY_H
#define TTENTRY_H

#include "bitboard.h"
#include "move.h"

typedef uint64_t ScoreInfo;

// TTEntry = 176 bits = 22o
class TTEntry
{
public:
    TTEntry();

private:
    ZKey m_zKey;       // Chess situation (64 bits)
    Move m;            // Best or refutation move (40 bits)
    ScoreInfo m_score; // All infos about the score and the score itself (64 bits)
    /* SCORE INFOS
     * | 32 bits |    2 bits     |  8 bits  | ...
     * | (score) | (Type of node)|  (Depth) | ...*/
    uchar m_age;       // The age of this entry (8 bits)
};

#endif // TTENTRY_H
