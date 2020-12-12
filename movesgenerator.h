#ifndef MOVESGENERATOR_H
#define MOVESGENERATOR_H

#include "move.h"
#include "engineerror.h"

class MovesGenerator
{
public:
    MovesGenerator();
    MovesGenerator(MovesGenerator const& mvGenToCopy);
    void clean();
    void cleanMovesForPly(int ply);
    bool hasNextMove(int ply);
    Move* getNextMove(int ply) noexcept(false);
    void resetToFirstMove(int ply) noexcept(false);
    void addToPly(int ply, const Square from, const Square to, const Flag &f);
    void addMove(Move* m);  // adding move to the movedPlayed
    void resetMovesPlayed();
    void deleteLastMovePlayed(Move* m = NULL) noexcept(false);

    int getCurrentPlayedMovesNb() const;
    int getCurrentMovesNb(int ply) const;
    int getCounterOfMoves(int ply) const;
    // !! Dangerous and is only used by constructor by copy !!
    Move* getMoveSearchTree(int ply, int mvCounter) const;
    Move* getMovesPlayed(int k) const;
    /*               NEVER USE THESE POINTERS               */

    ~MovesGenerator();

    static const int MAX_PLY = 20;    // moves simulated by IA
    static int MAX_PLY_IN_GAME;       // moves played during the game (dynamic variable)
    static const int MAX_MOVES = 256;
private:
    Move* m_movesSearchTree[MAX_PLY][MAX_MOVES]; // 20*256*4o = 20 ko
    std::vector<Move*> m_movesPlayed;
    int m_currentMovesNb[MAX_PLY]; // increment when adding a move to a certain ply.Decrement when cleaningMovesForPly.
    int m_counterOfMoves[MAX_PLY]; // increment and decrement by getNextMove and resetToFirstMove
    int m_currentPlayedMovesNb;    // increment when adding a move to the playedMove. Decrement when resetMovesPlayed or deleteLastMovePlayed

};

#endif // MOVESGENERATOR_H
