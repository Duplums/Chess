#include "engineerror.h"

const int EngineError::MOVE_NULL_PTR_EXCEPTION = 0;
const int EngineError::MOVE_TO_NOT_EMPTY = 1;
const int EngineError::MOVE_TO_EMPTY = 2;
const int EngineError::MOVE_FROM_EMPTY = 3;
const int EngineError::MOVE_EP_EMPTY = 4;
const int EngineError::CHESSBOARD_UNCONSISTANT = 5;
const int EngineError::MOVE_EMPTY_CAPTURE = 6;
const int EngineError::MOVE_GEN_INDEX_ERROR = 7;

EngineError::EngineError(const int &num, const std::string &description)  throw (): error_nb(num), description(description){
}
const char* EngineError::what() const throw (){
    std::stringstream err;
    err << "\n Erreur numéro " << error_nb << "\n" << description << "\n";
    return err.str().c_str();
}
