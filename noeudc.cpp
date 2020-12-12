#include "noeudc.h"

NoeudC::NoeudC(Square sq, Piece piece, Color c): m_sq(sq), m_piece(piece), m_movesBB(0), m_pinnedMovesBB(indicatrice[true]), m_color(c)
{}
NoeudC::NoeudC(const NoeudC &nodeToCopy){
    this->m_sq = nodeToCopy.getSquare();
    this->m_piece = nodeToCopy.getPiece();
    this->m_color = nodeToCopy.getColor();
    this->m_movesBB = nodeToCopy.getMovesBB();
    this->m_pinnedMovesBB = nodeToCopy.getPinnedMoves();
    this->m_neverUsed = nodeToCopy.isNeverUsed();
}
void NoeudC::setColor(Color c){
    m_color = c;
}
Color NoeudC::getColor() const{
    return m_color;
}
void NoeudC::addMove(Square sq){
    m_movesBB |= (one << sq);
}
void NoeudC::addMove(const BB m){
    m_movesBB |= m;
}
void NoeudC::setPinnedMove(const BB & m){
    m_pinnedMovesBB = m;
}
BB NoeudC::getPinnedMoves() const{
    return m_pinnedMovesBB;
}
void NoeudC::andMove(BB &bb){
    m_movesBB = bb & m_pinnedMovesBB;
    bb = m_movesBB;
}
BB NoeudC::getMovesBB() const{
    return m_movesBB;
}
bool NoeudC::hasNonEmptyMove(){
    return m_movesBB > 0;
}
Piece NoeudC::getPiece() const{
    return m_piece;
}
void NoeudC::setPiece(const Piece &piece){
    m_piece = piece;
}
void NoeudC::setNeverUsed(bool b){
    m_neverUsed = b;
}
bool NoeudC::isNeverUsed() const{
    return m_neverUsed;
}
Square NoeudC::getSquare() const{
    return m_sq;
}
void NoeudC::setSquare(const Square &sq){
    m_sq = sq;
}
void NoeudC::setMove(const BB &bb){
    m_movesBB = bb;
}
void NoeudC::cleanMovesBB(){
    m_movesBB = 0;
}
void NoeudC::cleanPinnedMovesBB(){
    m_pinnedMovesBB = indicatrice[true];
}
// Allocation of memory for the new move.
Move* NoeudC::getMoveFromAlgNot(std::string &m, bool white_is_playing, NoeudC* chessB[]){

    // Notation française
    std::regex std_notation("([RDFCT]?[a-h][1-8][x\s-][a-h][1-8](=?[DFCT][+#]?|[+#]|e\.p\.)?|O-O(-O)?)([+#])?");
    std::regex petit_roque("O-O([+#])?");
    std::regex gd_roque("O-O-O([+#])?");
    if(std::regex_match(m, std_notation)){
        // On fait un grand ou petit roque
        if(std::regex_match(m, petit_roque)){
            Square kingSq = (white_is_playing ? e1 : e8);
            if(chessB[kingSq] == NULL || chessB[kingSq]->getPiece() != king ||
               chessB[kingSq+2] == NULL  || chessB[kingSq+2]->getPiece() != rook){
                std::cout << "Petit roque impossible." << std::endl;
                return NULL;
            }
            return new Move(kingSq, (Square)(kingSq + 2), RIGHT_CASTLE_FLAG);
        } else if(std::regex_match(m, gd_roque)){
            Square kingSq = (white_is_playing ? e1 : e8);
            if(chessB[kingSq] == NULL || chessB[kingSq]->getPiece() != king ||
               chessB[kingSq-2] == NULL  || chessB[kingSq-2]->getPiece() != rook){
                std::cout << "Grand roque impossible." << std::endl;
                return NULL;
            }
            return new Move(kingSq, (Square)(kingSq - 2), LEFT_CASTLE_FLAG);
        } else {
            Square from, to;
            std::regex not_pawn("[RDFCT].*");
            std::regex ep(".*e\.p\.");
            std::regex promo(".*=[DFCT][+#]?");


            // On bouge autre chose qu'un pion (pas de promotion, ep, roque, double push)
            if(std::regex_match(m, not_pawn)){
                // 'a' == 97, '1' == 49
                from = (Square)static_cast<unsigned char>((m[1] - 'a') + ('8' - m[2])*8); // on ne manipule que des nbr positifs
                to = (Square)static_cast<unsigned char>((m[4] - 'a') + ('8' - m[5])*8);   // idem
            } else {
                from = (Square)static_cast<unsigned char>((m[0] - 'a') + ('8' - m[1])*8); // on ne manipule que des nbr positifs
                to = (Square)static_cast<unsigned char>((m[3] - 'a') + ('8' - m[4])*8);   // idem
            }
            if(chessB[from] == NULL){
                std::cout << "Pas de pièce au départ indiqué." << std::endl;
                return NULL;
            }
            // On fait une capture par autre chose qu'un pion
            if(std::regex_match(m, not_pawn) && m[3] == 'x'){
                if(chessB[to] == NULL){
                    std::cout << "Pas de pièce à prendre." << std::endl;
                    return NULL;
                } else{
                    return new Move(from, to, CAPTURE_FLAG[chessB[to]->getPiece()]);
                }
            } else if(std::regex_match(m, not_pawn)){
                if(chessB[to] != NULL){
                    std::cout << "La case d'arrivée est non vide." << std::endl;
                    return NULL;
                } else{
                    return new Move(from, to, QUIET_FLAG);
                }
            } else if(std::regex_match(m, ep)){
                // prise en passant d'un pion.
                if(chessB[from] == NULL || chessB[to] != NULL){
                    std::cout << "La case de départ est vide ou celle d'arrivée est non vide." << std::endl;
                    return NULL;
                }
                return new Move(from, to, EN_PASSANT_FLAG);
            } else if(std::regex_match(m, promo)){
                if(chessB[from] == NULL){
                    std::cout << "La case de départ est vide." << std::endl;
                    return NULL;
                }
                Piece toPiece = (chessB[to] == NULL ? empty : chessB[to]->getPiece());
                if(std::regex_match(m, std::regex(".*=D[+#]?")))
                     return new Move(from, to, PROMOTION_FLAG[toPiece][queen-rook]);
                else if(std::regex_match(m, std::regex(".*=F[+#]?")))
                     return new Move(from, to, PROMOTION_FLAG[toPiece][bishop-rook]);
                else if(std::regex_match(m, std::regex(".*=C[+#]?")))
                     return new Move(from, to, PROMOTION_FLAG[toPiece][knight-rook]);
                else if(std::regex_match(m, std::regex(".*=T[+#]?")))
                     return new Move(from, to, PROMOTION_FLAG[toPiece][rook-rook]);
            } else if (m[2] == 'x'){
                if(chessB[to] == NULL){
                    std::cout << "Pas de pièce à prendre." << std::endl;
                    return NULL;
                } else{
                    return new Move(from, to, CAPTURE_FLAG[chessB[to]->getPiece()]);
                }
            } else {
                if(chessB[to] != NULL){
                    std::cout << "La case d'arrivée est non vide." << std::endl;
                    return NULL;
                } else{
                    return new Move(from, to, QUIET_FLAG);
                }
            }
        }
    }else {
        std::cout << "Notation inconnue." << std::endl;
        return NULL;
    }
}


void NoeudC::toString(){
    std::cout << "Noeud n°" << (int)m_sq << " with piece " << m_piece << "\n";

}



