#ifndef IAERROR_H
#define IAERROR_H

#include<iostream>
#include<sstream>

class IAError: public std::exception
{
public:
    IAError(const int num = 0, const std::string description = "") throw();
    virtual const char* what() const throw();

    static const int DEFAULT_ERROR;
    static const int PV_ERROR;

private:
    std::string m_description;
    int m_error_nb;
};

#endif // IAERROR_H
