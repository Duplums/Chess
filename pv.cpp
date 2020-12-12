#include "pv.h"

PV::PV(int depthMax): m_depthMax(depthMax), DEPTH_MAX(depthMax)
{
    m_pvArray = new Move**[DEPTH_MAX]; // Allocating an array of array of move pointer (a line per ply)
    for(int k = 0; k < DEPTH_MAX; ++k){
        m_pvArray[k] = new Move*[DEPTH_MAX-k];
        for(int i = 0; i < DEPTH_MAX-k; ++i)
            m_pvArray[k][i] = new Move();
    }
}

// Returns just a copy of the best move (the callers can't change its value)
Move PV::getBestMove(){
    if(m_pvArray != NULL && m_pvArray[0] != NULL && m_pvArray[0][0] != NULL)
        return *m_pvArray[0][0];
    return Move();
}

std::vector<Move> PV::getPrincipalVariation(int &length){
    std::vector<Move> result(m_depthMax);
    if(m_pvArray != NULL && m_pvArray[0] != NULL){
        for(int k = 0; k < m_depthMax; ++k){
            if(m_pvArray[0][k] != NULL){
                result[k] = *m_pvArray[0][k];
            }else
                return result;
        }
    }
    return result;
}

void PV::setDepth(int depth){
    if(depth > DEPTH_MAX){ // reallocation of memory.
        DEPTH_MAX = depth;
        m_depthMax = depth;
        clean();
        m_pvArray = new Move**[DEPTH_MAX]; // Allocating an array of array of move pointer (a line per ply)
        for(int k = 0; k < DEPTH_MAX; ++k){
            m_pvArray[k] = new Move*[DEPTH_MAX-k];
            for(int i = 0; i < DEPTH_MAX-k; ++i)
                m_pvArray[k][i] = new Move();
        }
    }else{               // don't reallocate, juste erase all the data of the current pv.
        clean(false);
        m_depthMax = depth;
    }

}

// A PV is basically, if N is the max depth length :
//ply  maxLengthPV
//    +--------------------------------------------+
//0   |N                                           |
//    +------------------------------------------+-+
//1   |N-1                                       |
//    +----------------------------------------+-+
//2   |N-2                                     |
//    +--------------------------------------+-+
//3   |N-3                                   |
//    +------------------------------------+-+
//4   |N-4                                 |
//...                        /
//N-4 |4      |
//    +-----+-+
//N-3 |3    |
//    +---+-+
//N-2 |2  |
//    +-+-+
//N-1 |1|
//    +-+

// Set a move at the beginning of the line 'line' and copy the next line
// to this line.
// NB: Every move store is copied. So it's independant of the caller
void PV::setMoveToPly(int ply, Move* m){
    if(ply < 0 || ply >= m_depthMax)
        throw IAError(IAError::PV_ERROR, "Try to set move ply "+std::to_string(ply));
    m_pvArray[ply][0]->setMove(m);
    if(ply < m_depthMax-1)
        copyLine(ply+1, ply, 1);
}


// 0        | N                                    |
// ...
// (to)     | N-(to)                  |++++++++++++| <-
// (to)+1   | N-(to)-1               |+++++++++++++|
// ...
// (from)   | N-(from) (...)    |+++++++++++++++| <-
// (from)+1 | N-(from)-1 (...) |++++++++++++++++|
// ...

// copy all the (from) ply to the (to) ply using the (to)-shift
void PV::copyLine(int from, int to, int toShift){
    if((m_depthMax-to) < (m_depthMax-from)+toShift || from < 0 || from >= m_depthMax || to < 0 || to >= m_depthMax)
        throw IAError(IAError::PV_ERROR, "Copy error from "+std::to_string(from)+" to "+std::to_string(to)+" using shift "+std::to_string(toShift));
    else if(from != to){
        for(int k = 0; k < m_depthMax-from; ++k){
            m_pvArray[to][toShift+k]->setMove(m_pvArray[from][k]);
        }
    }
}

void PV::clean(bool deleteObj){
    for(int i = 0; i < DEPTH_MAX; ++i){
        for(int j = 0; j < DEPTH_MAX - i; ++j){
            if(deleteObj)
                delete m_pvArray[i][j];
            else if(m_pvArray[i][j] != NULL)
                m_pvArray[i][j]->clean();
        }
        if(deleteObj)
            delete[] m_pvArray[i];
    }
    if(deleteObj)
        delete[] m_pvArray;
}

std::string PV::toString(bool allInfos){

    std::string result("");
    for(int k = 0; k < m_depthMax; ++k){
        if(m_pvArray[0][k] == NULL){
            return "";
        }
        result += m_pvArray[0][k]->toString(allInfos) ;
        if(k < m_depthMax-1)
            result += " -> ";
    }
    return result;
}

PV::~PV(){
    clean();
}
