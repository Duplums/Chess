#ifndef MOVE_H
#define MOVE_H

#include<iostream>
#include <sstream>
#include <regex>
#include "bitboard.h"
#include "engineerror.h"

/* Déclaration des variables globales */

extern const Flag QUIET_FLAG;
extern const Flag DOUBLE_PUSH_FLAG;
extern const Flag DOUBLE_PUSH_SQUARE_FLAG[16];
extern const Flag EN_PASSANT_FLAG;
extern const Flag RIGHT_CASTLE_FLAG;
extern const Flag LEFT_CASTLE_FLAG;
extern const Flag CAPTURE_FLAG[6];
extern const Flag PROMOTION_FLAG[6][4];


// 1 move = 40 bits = 5o.
class Move
{
public:
    Move();
    Move(Square from, Square to, const Flag &flag);
    Move(const Move& m); // constructor by copy
    void setMove(const Move* m);
    void setMove(Square from, Square to, const Flag &flag);
    void setCastlingRights(CRights exCastlingRights);
    CRights getCastlingRights() const;
    void clean();
    bool enPassant();
    bool rightCastle();
    bool leftCastle();
    bool promotion();
    bool doublePush();
    bool capture();
    bool wasPieceNeverUsed();
    bool promoCapturedPiece();
    void pieceWasNeverUsed(bool b);
    bool isDoublePushSq();
    void setDoublePushSq(const Square& BBSq);
    Square getDoublePushSq();
    void pieceWasEnPassant(bool b);
    Square getEnPassantSq();
    Piece getPromotedPiece();
    Piece getCapturedPiece();
    Piece getPromoCapturedPiece();
    Square getFrom();
    Square getTo();
    MoveE getFlag() const;
    std::string toString(bool allInfos = true);

private:
    MoveE m_flag;
    CRights m_exCastlingRights;

    // Useful for generating a flag for a move
    /* 1 move = 32 bits, 1 flag = 20 bits

    | FROM/TO |                                    FLAG  (20 bits)                                                                 |
    from | to | promotion| capture | right castle | left castle | ep-capture | double pawn push | ex double push square | piece use
    (6)    (6)     (6)       (3)         (1)            (1)          (1)           (1)                    (5)                 (1)
    capture = 3 bits
    queen | bishop | knight | rook | pawn
    (101)   (100)    (011)   (010)  (001)
    promotion = 6 bits
       * * *  |   * * | *
    (captured) (promo) (isPromo)
    */
    static const Flag captureMask = 0x1c0000;
    static const Flag promotionMask = 0x1000;
    static const Flag promotedMask = 0x6000;
    static const Flag enPassantMask = 0x800000;
    static const Flag capturedPromoMask = 0x38000;
    static const Flag rightCastleMask = 0x200000;
    static const Flag leftCastleMask = 0x400000;
    static const Flag doublePushSquareMask = 0x3e000000;
    static const Flag pieceUseMask = 0x40000000;
    static const Flag doublePushMask = 0x1000000;
    static const Flag fromMask = 0x3f;
    static const Flag toMask = 0xfc0;
    static const Piece promotedPiece[4];
    static const Piece capturedPiece[6];
    static const Square doublePushSq[17];

    // Double push pawn
    static const char normalizedDoublePushSq = -24;

};

bool operator==(Move const& m1, Move const& m2);
bool operator!=(Move const& m1, Move const& m2);

#endif // MOVE_H
