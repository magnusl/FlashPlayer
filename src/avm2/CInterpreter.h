#ifndef _CINTERPRETER_H_
#define _CINTERPRETER_H_

#include "errorcode.h"
#include "Handle.h"
#include "CClassObject.h"
#include "CStringTable.h"
#include "ExecContext.h"

namespace avm2
{

namespace abc
{
class CAbcMethodDefinition;
class CAbcClassDefinition;
class CABCFile;
}

class CVirtualMachine;

/**
 * AVM2 bytecode interpreter.
 */
class CInterpreter
{
public:
    CInterpreter(CStringTable &, CVirtualMachine * vm);
    virtual ~CInterpreter();

    /**
     * Executes a method in the interpreter.
     *
     * \param _thisObject   The object that will used as the this object.
     * \param _globalObject The global object.
     * \param _timeout      The execution timeout, in milliseconds. A value of zero
     *      means that no timeout is enforced.
     * \param _currentClass The class the method belongs to.
     */
    ErrorCode_t execute(const abc::CAbcMethodDefinition *,
        Handle_t _thisObject,
        Handle_t _globalObject,
        uint32_t _timeout = 0,
        const abc::CAbcClassDefinition * _currentClass = 0);

private:

    //CClassObject * getClass(size_t _index);
    ErrorCode_t execute(ExecContext_t & ctx);

    Handle_t popOperand();      // pop operand from operand stack
    Handle_t popScope();        // pop object from scope stack
    void pushOperand(Handle_t); // push operand on the operand stack
    void PushScope(Handle_t);   // push scope on to the scope stack

    Handle_t searchScopeStack(const Object_MultiName &, ExecContext_t &);

private:
    Handle_t add(Handle_t _x, Handle_t _y);
    Handle_t addi(Handle_t _x, Handle_t _y);
    Handle_t subtract(Handle_t _x, Handle_t _y);
    Handle_t subtracti(Handle_t _x, Handle_t _y);
    Handle_t divide(Handle_t _x, Handle_t _y);

private:
    CStringTable &      stringTable;
    CVirtualMachine *   virtualMachine;
};

}

#endif
