#ifndef MOVESGENERATOR_H
#define MOVESGENERATOR_H

#include "move.h"
#include "engineerror.h"


class MovesGenerator
{
public:
    MovesGenerator();
    void clean();
    void cleanMovesForPly(int ply);
    bool hasNextMove(int ply);
    Move* getNextMove(int ply) throw(EngineError);
    void resetToFirstMove(int ply) throw(EngineError);
    void addToPly(int ply, const Square from, const Square to, const Flag &f);
    ~MovesGenerator();

    static const int MAX_PLY = 20;
    static const int MAX_MOVES = 256;
private:
    Move* m_movesSearchTree[MAX_PLY][MAX_MOVES]; // 20*256*4o = 20 ko
    int m_currentMovesNb[MAX_PLY];
    int m_counterOfMoves[MAX_PLY];
};

#endif // MOVESGENERATOR_H
