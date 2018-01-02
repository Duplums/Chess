#include "pv.h"

PV::PV(int depthMax): m_depthMax(depthMax)
{
    m_pvArray = new Move**[depthMax]; // Allocating an array of array of move pointer (a line per ply)
    for(int k = 0; k < depthMax; ++k){
        m_pvArray[k] = new Move*[depthMax-k];
        for(int i = 0; i < depthMax-k; ++i)
            m_pvArray[k][i] = new Move();
    }
}

void PV::setDepth(int depth){
    m_depthMax = depth;
    clean();
    m_pvArray = new Move**[m_depthMax]; // Allocating an array of array of move pointer (a line per ply)
    for(int k = 0; k < m_depthMax; ++k){
        m_pvArray[k] = new Move*[m_depthMax-k];
        for(int i = 0; i < m_depthMax-k; ++i)
            m_pvArray[k][i] = new Move();
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
void PV::setMoveToPly(int ply, Move* m) throw(IAError){
    if(ply < 0 || ply > m_depthMax)
        throw IAError(IAError::PV_ERROR, "Try to set move ply "+std::to_string(ply));
    m_pvArray[ply][0]->setMove(m);
    if(ply < m_depthMax)
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
void PV::copyLine(int from, int to, int toShift) throw(IAError){
    if((m_depthMax-to) < (m_depthMax-from)+toShift || from < 0 || from >= m_depthMax || to < 0 || to >= m_depthMax)
        throw IAError(IAError::PV_ERROR, "Copy error from "+std::to_string(from)+" to "+std::to_string(to)+" using shift "+std::to_string(toShift));
    else if(from != to){
        for(int k = 0; k < m_depthMax-from; ++k){
            m_pvArray[to][toShift+k]->setMove(m_pvArray[from][k]);
        }
    }
}

void PV::clean(){
    for(int i = 0; i < m_depthMax; ++i){
        for(int j = 0; j < m_depthMax - i; ++j)
            delete m_pvArray[i][j];
        delete[] m_pvArray[i];
    }
    delete[] m_pvArray;
}
PV::~PV(){
    clean();
}
