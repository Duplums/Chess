#include "ia.h"

IA::IA(ChessBoard* board, const int depth): m_chessboard(board), m_depthMaxSearch(depth), m_PV(depth)
{}

void IA::setDepthMaxSearch(int d){
    m_depthMaxSearch = d;
    m_PV.setDepth(d);
}

// the IA plays against itself
void IA::playAGame(int nbPly){
    for(int ply = 0; ply < nbPly; ++ply){
        searchBestMove();
        Move bestMove = m_PV.getBestMove();
        m_chessboard->playMove(&bestMove);
    }
}

void IA::searchBestMove(){
    m_chessboard->toString();
    int nbNodesEval = 0, nbNodes = 0, nbCuts = 0;
    int eval = alphaBeta(-10, 10, m_depthMaxSearch, 0, nbNodesEval, nbNodes, nbCuts);
    std::cout << "Best move found : " << m_PV.toString(false) << std::endl;
    std::cout << "Number of nodes evaluated : " << nbNodesEval << std::endl;
    std::cout << "Number of total nodes : " << nbNodes << std::endl;
    std::cout << "Number of cuts : " << nbCuts << std::endl;
    std::cout << "Evaluation of the best move (prof "<< m_depthMaxSearch << ") : " << eval << std::endl;
}




int IA::alphaBeta(int alpha, int beta, int depth, int ply, int &nbNodesEval, int &nbNodes, int &nbCuts) noexcept(false){
    if(depth == 0) {
        nbNodesEval++;
        return quiescence(alpha, beta);
    }
    m_chessboard->generateLegalMoves(ply); // it changes 'plySearch'
    Move* moveToPlay = NULL;
    int score = 0;
    while(m_chessboard->hasNextMove(ply)){
        nbNodes++;
        moveToPlay = m_chessboard->getNextMove(ply);
        m_chessboard->makeMove(moveToPlay);
        score = -alphaBeta(-beta, -alpha, depth-1, ply+1, nbNodesEval, nbNodes, nbCuts);
        m_chessboard->unmakeMove(moveToPlay);
        m_chessboard->setPlySearch(ply);

        if(score >= beta) {
            nbCuts++;
            return beta;
        }
        if(score > alpha){
            alpha = score;
            m_PV.setMoveToPly(ply, moveToPlay); // copy every moves from (ply+1) to (ply) and add moveToPlay.
        }
    }
    return alpha;

}

int IA::quiescence(int alpha, int beta){
    return 2 * m_chessboard->getColorToPlay() - 1;
}
