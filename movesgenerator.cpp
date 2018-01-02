#include "movesgenerator.h"

MovesGenerator::MovesGenerator()
{
    for(int k = 0; k < MAX_PLY; ++k){
        for(int i = 0; i < MAX_MOVES; ++i)
            m_movesSearchTree[k][i] = new Move();
        m_currentMovesNb[k] = 0;
        m_counterOfMoves[k] = 0;
    }
}

void MovesGenerator::cleanMovesForPly(int ply){
    resetToFirstMove(ply);
    m_currentMovesNb[ply] = 0;
    for(int k = 0; k < MAX_MOVES; ++k)
        m_movesSearchTree[ply][k]->clean();
}

void MovesGenerator::addToPly(int ply, const Square from, const Square to, const Flag &f){
    m_movesSearchTree[ply][m_currentMovesNb[ply]++]->setMove(from, to, f);
    std::cout << "add to ply " << ply << " move " << m_movesSearchTree[ply][m_currentMovesNb[ply]]->toString();
}

Move* MovesGenerator::getNextMove(int ply) throw(EngineError){
    if(ply < 0 || ply > MAX_PLY || m_counterOfMoves[ply] >= m_currentMovesNb[ply])
        throw EngineError(EngineError::MOVE_GEN_INDEX_ERROR, "Try to get next move for ply "+ply);
    return m_movesSearchTree[ply][m_counterOfMoves[ply]++];
}

void MovesGenerator::resetToFirstMove(int ply) throw(EngineError){
    if(ply < 0 || ply > MAX_PLY)
        throw EngineError(EngineError::MOVE_GEN_INDEX_ERROR, "Try to reset first move for ply "+ply);
    m_counterOfMoves[ply] = 0;
}

bool MovesGenerator::hasNextMove(int ply){
    return m_counterOfMoves[ply] < m_currentMovesNb[ply];
}

void MovesGenerator::clean(){
    for(int ply = 0; ply < MAX_PLY; ++ply)
        cleanMovesForPly(ply);
}

MovesGenerator::~MovesGenerator(){
    for(int k = 0; k < MAX_PLY; ++k){
        for(int i = 0; i < MAX_MOVES; ++i)
            delete m_movesSearchTree[k][i];
    }
}
