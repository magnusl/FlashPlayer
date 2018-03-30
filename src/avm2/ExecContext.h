#ifndef _EXECCONTEXT_H_
#define _EXECCONTEXT_H_

#include <vector>
#include "Handle.h"
#include "CAbcClassDefinition.h"
#include "CAbcMethodDefinition.h"

namespace avm2
{

/**
 * Execution context
 */
struct ExecContext_t
{
    inline Atom_t getRegister(size_t index)
    {
        if (index >= registers.size()) {
            throw std::runtime_error("invalid register index.");
        }
        return registers[index];
    }

    inline void setRegister(size_t index, Atom_t value)
    {
        if (index >= registers.size()) {
            throw std::runtime_error("invalid register index.");
        }
        registers[index] = value;
    }

    std::vector<Handle_t>   operandStack;
    std::vector<Handle_t>   scopeStack;
    std::vector<Handle_t>   registers;

    /////
    // Stack pointers
    size_t  opStackPtr;     /**< operand stack pointer */
    size_t  scopeStackPtr;  /**< scope stack pointer */

    /////
    // Limits
    size_t  maxScopeStack;
    size_t  maxOperandStack;
    size_t  numRegisters;

    /////
    // The current class we are executing in.
    const abc::CAbcClassDefinition * currentClass;

    /////
    // The current method we are executing
    const abc::CAbcMethodDefinition * currentMethod;

    /////
    // The global object for this context
    Handle_t globalObject;
};

}

#endif