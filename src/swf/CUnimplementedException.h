#ifndef _CPARSEEXCEPTION_H_
#define _CPARSEEXCEPTION_H_

#include <stdexcept>

namespace swf
{

/**
 * Exception indicating that a SWF feature isn't implemented.
 */
class CUnimplementedException : public std::runtime_error
{
public:
    CUnimplementedException(const char * _message) : std::runtime_error(_message)
    {
    }
    CUnimplementedException(const std::string & _message) : std::runtime_error(_message)
    {
    }
};

}

#endif