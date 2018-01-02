#include "ia.h"

IA::IA(ChessBoard &board, const int depth): m_chessboard(&board), m_depthSearch(depth), m_PV(depth)
{}

void IA::setDepthSearch(int d){
    m_depthSearch = d;
    m_PV.setDepth(d);
}

int IA::alphaBeta(int alpha, int beta, int depth, int ply) throw(EngineError){
    if(depth == 0) return quiescence(alpha, beta);
    m_chessboard->generateLegalMoves(ply);
    Move* moveToPlay = NULL;
    int score = 0;
    while(m_chessboard->hasNextMove(ply)){
        moveToPlay = m_chessboard->getNextMove(ply);
        m_chessboard->makeMove(moveToPlay);
        score = -alphaBeta(-beta, -alpha, depth-1, ply+1);
        m_chessboard->unmakeMove(moveToPlay);

        if(score >= beta) return beta;

        if(score > alpha){
            alpha = score;
            m_PV.setMoveToPly(ply, moveToPlay);
        }
    }
    return alpha;

}

int IA::quiescence(int alpha, int beta){
    return 0;
}
