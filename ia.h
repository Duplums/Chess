#ifndef IA_H
#define IA_H

#include "chessboard.h"
#include "transpositiontable.h"
#include "pv.h"

class IA
{
public:
    IA(ChessBoard *board, const int depth = 0);
    void playAGame(int ply);
    void searchBestMove();
    int alphaBeta(int alpha, int beta, int depth, int ply, int &nbNodesEval, int &nbNodes, int &nbCuts) noexcept(false);
    void setDepthMaxSearch(int d);
    int quiescence(int a, int b);

private:
    ChessBoard* m_chessboard = NULL;
    PV m_PV;
    int m_depthMaxSearch;
    TranspositionTable* m_table = NULL;

};

#endif // IA_H
