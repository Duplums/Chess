#ifndef ENGINEERROR_H
#define ENGINEERROR_H

#include<iostream>
#include<sstream>
class EngineError: public std::exception
{
public:
    EngineError(const int& num = -1, std::string const& description = std::string()) throw();
    virtual const char* what() const throw();

    static const int MOVE_NULL_PTR_EXCEPTION;
    static const int MOVE_TO_NOT_EMPTY;
    static const int MOVE_TO_EMPTY;
    static const int MOVE_FROM_EMPTY;
    static const int MOVE_EP_EMPTY;
    static const int CHESSBOARD_UNCONSISTANT;
    static const int MOVE_EMPTY_CAPTURE;
    static const int MOVE_GEN_INDEX_ERROR;
    static const int MOVE_GEN_OBJ_ERROR;

private:
    int error_nb;
    std::string description;
};

#endif // ENGINEERROR_H
