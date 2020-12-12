#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#define SIZE_ECHIQUIER 8

#include "move.h"
#include "noeudc.h"
#include "engineerror.h"
#include "movesgenerator.h"
#include "zobristhash.h"
#include "pv.h"
#include <QDebug>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>
#include <cctype>
#include<fstream>
#include <ctime>
#include <cstdio>

class ChessBoard
{
public:
    ChessBoard();
    ChessBoard(ChessBoard const& boardToCopy); // deep copy of a chessboard (useful for multithreading)
    void clean(bool deletePointers = false);
    bool initChessBoard();
    bool initZKeyFromScratch();
    bool updateZKey(Move* m);
    bool setPosition(const std::string& pos, bool cleanInitAndDeletePointers = false);
    void setDepthMax(int depth);
    bool testPosition(int k);
    Move* getNextMove(int ply);
    bool hasNextMove(int ply);
    void resetToFirstMove(int ply);
    void setPlySearch(int ply);
    void setToPV(int ply, Move*m);
    void moveGenTest(uint &depth);
    void moveGenRec(std::ofstream* file, uchar depth, uint& nodes, uint &totalNodes, uint& captures, uint& ep, uint& castles, uint& promo, uint& check, uint& checkMate);
    void makeMove(std::string); // TODO
    void unmakeMove(std::string); // TODO
    void playMove(Move *);
    void undoMove(Move*);
    void makeMove(Move*) noexcept(false);
    void unmakeMove(Move*) noexcept(false);
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

    // Getters and destructor
    ZKey getZobristKey() const;
    BB getChessBB() const;
    Color getColorToPlay() const;
    int getPlySearch() const;
    int getMaxDepth() const;
    MovesGenerator *getMovesGen() const;
    int getIndexDeadPieces(Color c) const;
    BB getChessBBPerColor(Color c) const;
    Square getDoublePushSq(Color c) const;
    BB getChessBBPerColorAndKind(Color c, PieceShift kind) const;
    NoeudC* getDeadPiece(Color c, int k) const;
    NoeudC* getChessBPiece(Square sq) const;
    CRights getCastlingRights() const;

    ~ChessBoard();

private:
    Color m_whiteIsPlaying;
    NoeudC* m_chessB[64];
    NoeudC* m_deadPiecesBB[2][16];
    uchar m_indexDeadPieces[2];
    BB m_chessBB_[2];       // All pieces per color
    BB m_chessBB[2][6];     // All pieces per kind and per color
    BB m_chessBoard;
    Square m_doublePushSq[2]; // square of the pawn that double push per color (possible ep captured)
    MovesGenerator* m_movesGen;
    int m_plySearch;
    unsigned int m_depthMax;
    ZKey m_zKey;        // zobristKey of this chessboard.
    CRights m_castlingRights;
    // About CastlingRights: 4 bits <-> (1) (2) (3) (4) ****
    // (1) = right castle for white
    // (2) = left castle for white
    // (3) = right castle for black
    // (4) = left castle for black
    static const char doublePushPawn[2]; // Useful to determine the double push square from the init line of the pawn.
    static const char doublePushEPSq[2]; // Useful to determine the EP square from the double push square.

};

#endif // CHESSBOARD_H
