#include "movesgenerator.h"

int MovesGenerator::MAX_PLY_IN_GAME = 200;

MovesGenerator::MovesGenerator(): m_movesPlayed(MAX_PLY_IN_GAME)
{
    m_currentPlayedMovesNb = 0;
    for(int k = 0; k < MAX_PLY; ++k){
        for(int i = 0; i < MAX_MOVES; ++i)
            m_movesSearchTree[k][i] = new Move();
        m_currentMovesNb[k] = 0;
        m_counterOfMoves[k] = 0;
    }
    for(int k = 0; k < MAX_PLY_IN_GAME; ++k)
        m_movesPlayed[k] = new Move();
}


MovesGenerator::MovesGenerator(const MovesGenerator &mvGenToCopy){
    this->m_currentPlayedMovesNb = mvGenToCopy.getCurrentPlayedMovesNb();
    for(int ply = 0; ply < MovesGenerator::MAX_PLY; ++ply){
        this->m_currentMovesNb[ply] = mvGenToCopy.getCurrentMovesNb(ply);
        this->m_counterOfMoves[ply] = mvGenToCopy.getCounterOfMoves(ply);
    }

    for(int ply = 0; ply < MovesGenerator::MAX_PLY; ++ply){
        for(int mvCounter = 0; mvCounter < MovesGenerator::MAX_MOVES; ++mvCounter){
            this->m_movesSearchTree[ply][mvCounter] = new Move(*mvGenToCopy.getMoveSearchTree(ply, mvCounter));
        }
    }
    this->m_movesPlayed.resize(MovesGenerator::MAX_PLY_IN_GAME);
    for(int k = 0; k < MovesGenerator::MAX_PLY_IN_GAME; ++k)
        this->m_movesPlayed[k] = new Move(*mvGenToCopy.getMovesPlayed(k));
}

void MovesGenerator::cleanMovesForPly(int ply){
    resetToFirstMove(ply);
    m_currentMovesNb[ply] = 0;
    for(int k = 0; k < MAX_MOVES; ++k)
        m_movesSearchTree[ply][k]->clean();
}

void MovesGenerator::addMove(Move *m){
    if(m_currentPlayedMovesNb >= MAX_PLY_IN_GAME){
        // Reallocate memory (rare)
        m_movesPlayed.resize(MAX_PLY_IN_GAME+100);
        for(int k = 0; k < 100; ++k)
            m_movesPlayed[k+MAX_PLY_IN_GAME] = new Move();
        MAX_PLY_IN_GAME += 100;
    }
    m_movesPlayed[m_currentPlayedMovesNb++]->setMove(m);
}

void MovesGenerator::resetMovesPlayed(){
    for(int k = 0; k < MAX_PLY_IN_GAME; ++k)
        m_movesPlayed[k]->clean();
    m_currentPlayedMovesNb = 0;
}

void MovesGenerator::deleteLastMovePlayed(Move *m) noexcept(false){
    if(m_currentPlayedMovesNb < 1)
        throw EngineError(EngineError::MOVE_GEN_INDEX_ERROR, "Try to undo a move despite counter = "+m_currentPlayedMovesNb);
    if(m != NULL){
        // Compare the move to the last move.
        if(*m_movesPlayed[m_currentPlayedMovesNb-1] != *m)
            throw EngineError(EngineError::MOVE_GEN_OBJ_ERROR, "Try to undo a move different to the last move");
    }
    m_movesPlayed[--m_currentPlayedMovesNb]->clean();
}

void MovesGenerator::addToPly(int ply, const Square from, const Square to, const Flag &f){
    m_movesSearchTree[ply][m_currentMovesNb[ply]++]->setMove(from, to, f);
}

Move* MovesGenerator::getNextMove(int ply) noexcept(false){
    if(ply < 0 || ply > MAX_PLY || m_counterOfMoves[ply] >= m_currentMovesNb[ply])
        throw EngineError(EngineError::MOVE_GEN_INDEX_ERROR, "Try to get next move for ply "+ply);
    return m_movesSearchTree[ply][m_counterOfMoves[ply]++];
}

void MovesGenerator::resetToFirstMove(int ply) noexcept(false){
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

// Getters and destructors

int MovesGenerator::getCurrentPlayedMovesNb() const{
    return m_currentPlayedMovesNb;
}

int MovesGenerator::getCurrentMovesNb(int ply) const{
    if(ply < 0 || ply >= MAX_PLY){
        std::cout << "Warning: get current moves number for the ply " << ply << ". Returned 0" << std::endl;
        return 0;
    }
    return m_currentMovesNb[ply];
}

int MovesGenerator::getCounterOfMoves(int ply) const{
    if(ply < 0 || ply >= MAX_PLY){
        std::cout << "Warning: get counters of moves for the ply " << ply << ". Returned 0" << std::endl;
        return 0;
    }
    return m_counterOfMoves[ply];
}

Move* MovesGenerator::getMoveSearchTree(int ply, int mvCounter) const{
    if(ply < 0 || ply >= MAX_PLY || mvCounter < 0 || mvCounter >= MAX_MOVES){
        std::cout << "Warning: getMoveSearchTree for the ply " << ply << " and mv counter " << mvCounter << ". Returned null" << std::endl;
        return NULL;
    }
    return m_movesSearchTree[ply][mvCounter];
}

Move* MovesGenerator::getMovesPlayed(int k) const{
    if(k < 0 || k >= MAX_PLY_IN_GAME){
        std::cout << "Warning: getMovesPlayed for the ply " << k << ". Returned null" << std::endl;
        return NULL;
    }
    return m_movesPlayed[k];
}

MovesGenerator::~MovesGenerator(){
    for(int k = 0; k < MAX_PLY; ++k){
        for(int i = 0; i < MAX_MOVES; ++i)
            delete m_movesSearchTree[k][i];
    }
    for(int k = 0; k < MAX_PLY_IN_GAME; ++k)
        delete m_movesPlayed[k];
}
