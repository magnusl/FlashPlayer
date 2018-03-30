#ifndef _CPARSEEXCEPTION_H_
#define _CPARSEEXCEPTION_H_

#include <stdexcept>

namespace avm2
{

/**
 * Exception indicating a parse error.
 */
class CParseException : public std::runtime_error
{
public:
    CParseException(const char * _message) : std::runtime_error(_message)
    {
    }
    CParseException(const std::string & _message) : std::runtime_error(_message)
    {
    }
};

}

#endif