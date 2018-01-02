#include "iaerror.h"

const int IAError::DEFAULT_ERROR = -1;
const int IAError::PV_ERROR = 1;

IAError::IAError(const int num, const std::string description) throw(): m_error_nb(num), m_description(description)
{
}

const char* IAError::what() const throw (){
    std::stringstream err;
    err << "\n Erreur numéro " << m_error_nb << "\n" << m_description << "\n";
    return err.str().c_str();
}
