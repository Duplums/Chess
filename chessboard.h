#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#define SIZE_ECHIQUIER 8

#include "move.h"
#include "noeudc.h"
#include "engineerror.h"
#include "movesgenerator.h"
#include "pv.h"
#include <QDebug>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>
#include <cctype>

class ChessBoard
{
public:
    ChessBoard();
    void clean();
    bool initChessBoard();
    bool setPosition(const std::string& pos);
    bool testPosition(int k);
    Move* getNextMove(int ply);
    bool hasNextMove(int ply);
    void resetToFirstMove(int ply);
    void setPlySearch(int ply);
    void setToPV(int ply, Move*m);
    void moveGenTest(uint &depth);
    void moveGenRec(uchar depth, uint& nodes, uint &totalNodes, uint& captures, uint& ep, uint& castles, uint& promo, uint& check, uint& checkMate);
    void makeMove(std::string);
    void unmakeMove(std::string);
    void makeMove(Move*) throw (EngineError);
    void unmakeMove(Move*) throw (EngineError);
    void cleanMovesOf(const Color &c);
    void generateVectorizedMoves(NoeudC* piece, const bool &rightCastle, const bool &leftCastle, const bool &enPassant, const bool &promotion, const bool &doublePush);
    bool generateLegalMoves(int ply = 0);
    void analysePiecesAdjAvecClouage(Square sq, Color c, BB& pinnedAllyPieces, BB& ennemyAdjBB);
    bool squareIsAttacked(Square sq, Color c);
    void bishopAttacksBB(Square sq, BB& result);
    void rookAttacksBB(Square sq, BB& result);
    void queenAttacksBB(Square sq, BB& result);
    void split(const std::string& s, const std::regex& r, std::vector<std::string>& result);

    void toString();
    ~ChessBoard();

private:
    Color m_whiteIsPlaying;
    NoeudC* m_chessB[64];
    NoeudC* m_deadPiecesBB[2][16];
    uchar m_indexDeadPieces[2];
    BB m_chessBB_[2];       // All pieces per color
    BB m_chessBB[2][6];     // All pieces per kind and per color
    BB m_chessBoard;
    BB m_possibleEPPieces[2];
    MovesGenerator m_movesGen;
    int m_plySearch;
    BitBoard m_bitboard;
    unsigned int m_depthMax;

};

#endif // CHESSBOARD_H
