#include "move.h"


/* Initialisation des variables globales
from | to | promotion| capture | right castle | left castle | ep-capture | double pawn push | ex double push square | piece use |
(6)    (6)     (6)       (3)         (1)            (1)          (1)           (1)                (5)                    (1)
*/
const Flag QUIET_FLAG = 0;
const Flag DOUBLE_PUSH_FLAG = one << 12;
const Flag EN_PASSANT_FLAG = one << 11;
const Flag DOUBLE_PUSH_SQUARE_FLAG[16] = {
    // ancien sq du pion qui a double-push côté noirs (5e ligne)
    1 << 13, 2 << 13, 3 << 13, 4 << 13, 5 << 13, 6 << 13, 7 << 13, 8 << 13,
    // ancien sq du pion qui a double-push côté blanc (4e ligne)
    9 << 13, 10 << 13, 11 << 13, 12 << 13, 13 << 13, 14 << 13, 15 << 13, 16 << 13
};
const Flag RIGHT_CASTLE_FLAG = one << 9;
const Flag LEFT_CASTLE_FLAG = one << 10;
const Flag CAPTURE_FLAG[6] = {
// empty | pawn | rook | knight | bishop | queen
    0, 1 << 6, 2 << 6, 3 << 6, 4 << 6, 5 << 6
};
const Flag PROMOTION_FLAG[6][4] = {
// rook | knight | bishop | queen promotion
    {1, 3, 5, 7}, // empty captured
    {9, 11, 13, 15}, // pawn captured
    {17, 19, 21, 23}, // rook captured
    {25, 27, 29, 31}, // knight captured
    {33, 35, 37, 39}, // bishop captured
    {41, 43, 45, 47}, // queen captured
}; // 1st: promoted piece, 2nd: captured piece

const Piece promotedPiece[4] = {rook, knight, bishop, queen};
const Piece capturedPiece[6] = {empty, pawn, rook, knight, bishop, queen};
// Double push pawn
const char doublePushPawn[2] = {16, -16};
const char normalizedDoublePushSq = -24;

Move::Move(Square from, Square to, const Flag& flag)
{
    m_flag = from | (to << 6) | (flag << 12);
}

Move::Move(const Move &m): m_flag(m.getFlag())
{}

Move::Move(): m_flag(0)
{}

void Move::setMove(const Move *m){
    m_flag = m->getFlag();
}
void Move::setMove(Square from, Square to, const Flag& flag){
    m_flag = from | (to << 6) | (flag << 12);
}
void Move::clean(){
    m_flag = 0;
}
bool Move::capture(){
    return (m_flag & captureMask) > 0;
}
bool Move::doublePush(){
    return (m_flag & doublePushMask) > 0;
}
bool Move::leftCastle(){
    return (m_flag & leftCastleMask) > 0;
}
bool Move::rightCastle(){
    return (m_flag & rightCastleMask) > 0;
}
bool Move::promotion(){
    return (m_flag & promotionMask) > 0;
}
bool Move::enPassant(){
    return (m_flag & enPassantMask) > 0;
}
Square Move::getEnPassantSq(){
    return (Square)(8 * (getFrom()/8) + getTo()%8);
}
void Move::pieceWasNeverUsed(bool b){
    m_flag = (m_flag & ~(one << 30)) | (b << 30);
}
void Move::addToFlag(const Flag &f){
    m_flag |= (f << 12);
}
bool Move::wasPieceNeverUsed(){
    return (m_flag & pieceUseMask) > 0;
}
Square Move::getSquareEPCapturedPiece(){
    if((m_flag & doublePushSquareMask) > 0){
        return (Square) (((m_flag & doublePushSquareMask) >> 25) + 23);
    }
    return outside;
}
Piece Move::getPromotedPiece(){
    return promotedPiece[(m_flag & promotedMask) >> 13];
}
Piece Move::getPromoCapturedPiece(){
    return capturedPiece[(m_flag & capturedPromoMask) >> 15];
}
Piece Move::getCapturedPiece(){
    return capturedPiece[(m_flag & captureMask) >> 18];
}
Square Move::getFrom(){
    return (Square)(m_flag & fromMask);
}
Square Move::getTo(){
    return (Square)((m_flag & toMask) >> 6);
}
MoveE Move::getFlag() const{
    return m_flag;
}

bool operator==(Move const& m1, Move const& m2){
    return m1.getFlag() == m2.getFlag();
}

std::string Move::toString(bool allInfos){
    std::stringstream result;
    if(allInfos)
        result    << "Move from " << BitBoard::toString(getFrom()) << "[" << (int)getFrom() << "]"
                  << " to " << BitBoard::toString(getTo())  << "[" << (int)getTo() << "]"
                  << " with captured " << capture()
                  << " RC " << rightCastle() << " LC " << leftCastle()
                  << " promotion " << promotion() << " EP " << enPassant()
                  << " double push " << doublePush()
                  << " ex double push sq " << BitBoard::toString(getSquareEPCapturedPiece()) << std::endl;
    else
        result << BitBoard::toString(getFrom()) << BitBoard::toString(getTo());
    return result.str();
}
