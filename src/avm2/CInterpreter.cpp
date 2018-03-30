#include "CInterpreter.h"
#include "CAbcMethodDefinition.h"
#include "CAbcClassDefinition.h"
#include "CMemIO.h"
#include "opcodes.h"
#include "CABCFile.h"
#include "typeconv.h"
#include "ExecContext.h"
#include "CScriptObject.h"
#include "CVirtualMachine.h"
#include "CABCClassObject.h"
#include "CClassObject.h"

#include <stdexcept>
#include <iostream>

using namespace std;

namespace avm2
{

CInterpreter::CInterpreter(CStringTable & strTable, CVirtualMachine * vm) : 
    stringTable(strTable), virtualMachine(vm)
{
}

CInterpreter::~CInterpreter()
{
}

/**
 * Initializes the execution environment so that the
 * method can be executed.
 */
ErrorCode_t CInterpreter::execute(const abc::CAbcMethodDefinition * _method,
    Handle_t _thisObject,
    Handle_t _globalObject,
    uint32_t _timeout,
    const abc::CAbcClassDefinition * _currentClass)
{
    ExecContext_t ctx;
    ctx.currentClass        = _currentClass;
    ctx.currentMethod       = _method;
    ctx.globalObject        = _globalObject;
    ctx.maxOperandStack     = _method->max_stack;
    ctx.maxScopeStack       = _method->max_scope_depth;
    ctx.numRegisters        = _method->local_count;

    // resize the stacks + registers
    ctx.operandStack.resize(ctx.maxOperandStack);
    ctx.scopeStack.resize(ctx.maxScopeStack);
    ctx.registers.resize(ctx.numRegisters);

    // set initial stack pointers
    ctx.opStackPtr          = 0;
    ctx.scopeStackPtr       = 0;
    // set registers
    ctx.registers[0]        = _thisObject;  // register 0 is always the this object

    // set the initial value of the registers to 'undefined'
    for(size_t i = 1; i < ctx.numRegisters; ++i) {
        ctx.registers[i] = Undefined_atom;
    }
    return execute(ctx);
}

inline void ExpectStackSize(size_t actual, size_t required)
{
    if (actual < required) {
        throw std::runtime_error("Stack underflow.");
    }
}

inline void ExpectNotFull(size_t maxSize, size_t actual)
{
    if (actual >= maxSize) {
        throw std::runtime_error("Stack overflow.");
    }
}

/**
 * Searches the scope stack for a object which has a matching property.
 */
Handle_t CInterpreter::searchScopeStack(const Object_MultiName & name,
                                        ExecContext_t & context)
{
    /////
    // Search the scope stack for a object which has matching property.
    if (context.opStackPtr) {
        for(size_t i = context.opStackPtr; i > 0; --i) {
            if (CScriptObject * obj = toObject(context.scopeStack[context.opStackPtr - 1])) {
                if (obj->hasProperty(name, context.currentClass)) {
                    return context.scopeStack[context.opStackPtr - 1];
                }
            } else {
                // Not a object
                throw runtime_error("Item on the scope stack isn't a object.");
            }
        }
    }
    /////
    // Check if the global object has a matching object.
    if (CScriptObject * global = toObject(context.globalObject)) {
        if (global->hasProperty(name, context.currentClass)) {
            return context.globalObject;
        }
        // Check if there is a matching object in a package.
        if (name.Type == Object_MultiName::eQualifiedName) {
            if (CClassObject * obj = virtualMachine->findClass(name.qualifiedName))
            {
                global->Put(name.qualifiedName, obj);
                return global;
            }
        }
    }
    return objToAtom(0);
}

/**
 * Executes the raw AVM2 bytecode directly from the source.
 */
ErrorCode_t CInterpreter::execute(ExecContext_t & ctx)
{
    const abc::CABCFile * abc = ctx.currentMethod->delivery;
    const avm2::abc::CConstantPool & pool = abc->GetConstantPool();
    const std::vector<uint8_t> * bc = ctx.currentMethod->byteCode;
    ErrorCode_t err;

    avm2::io::CMemIO io(*bc);
    while(true) 
    {
        uint8_t b = io.GetU8();
        Opcode_t opcode = (Opcode_t) b;
        switch(opcode) 
        {
            /**************************************************************************/
            /*                          Scope stack management                        */
            /**************************************************************************/
        case eAVM2_pushscope:
            {
                /////
                // Guard and verify the stack pointers before continuing.
                ExpectStackSize(ctx.opStackPtr, 1);
                ExpectNotFull(ctx.maxScopeStack, ctx.scopeStackPtr);

                Handle_t op = ctx.operandStack[--ctx.opStackPtr];
                // make sure that the scope stack isn't full
                ctx.scopeStack[ctx.scopeStackPtr++] = op;
                break;
            }
        case eAVM2_popscope:
            {
                /////
                // Pop a object of the scope stack
                ExpectStackSize(ctx.opStackPtr, 1);
                --ctx.opStackPtr;
                break;
            }
        case eAVM2_getscopeobject:
            {
                /////
                // Push the first object on the scope stack onto the operand stack
                ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);     /**< make sure the operand stack isn't full */
                uint8_t index = io.GetU8();
                if (index >= ctx.scopeStackPtr) {
                    throw std::runtime_error("getscopeobject: invalid index.");
                }
                ctx.operandStack[ctx.opStackPtr++] = ctx.scopeStack[index];
                break;
            }
            /**************************************************************************/
            /*                              Property search                           */
            /**************************************************************************/
        case eAVM2_findpropery:
        case eAVM2_findpropstrict:
            {
                ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);
                Handle_t objHandle = searchScopeStack(abc->GetMultiName(io.GetU30()), ctx);
                if (CScriptObject * obj = toObject(objHandle)) {
                    // Found a matching object, push it on the operand stack
                } else {
                    // No matching object, check if it's exported from a script.
                    assert(false);

                    if (opcode == eAVM2_findpropery) {
                        // Push the global object onto the stack
                    } else {
                        // ReferenceError.
                    }
                }
                break;
            }
        case eAVM2_getlex: /** find and get a property */
            {
                ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);

                Object_MultiName propName = abc->GetMultiName(io.GetU30());

                if (const char * name = stringTable.get(propName.qualifiedName.Name))
                {
                    std::cout << "getlex: '" << name << "'" << std::endl;
                }

                Handle_t objHandle = searchScopeStack(propName, ctx);
                if (CScriptObject * obj = toObject(objHandle)) {
                    Handle_t propValue;
                    if (obj->Get(propName, propValue, ctx.currentClass)) {
                        ctx.operandStack[ctx.opStackPtr++] = propValue; /**< push property value on stack */
                    } else {
                        // ReferenceError
                        assert(false);
                    }
                } else {
                    // Check scripts for exports.
                    assert(false);
                }
                break;
            }
        case eAVM2_getproperty: 
            {
                /////
                // Guard and verify the stack pointers before continuing.
                ExpectStackSize(ctx.opStackPtr, 1);

                Object_MultiName name = abc->GetMultiName(io.GetU30());
                Handle_t op = ctx.operandStack[--ctx.opStackPtr];
                if (CScriptObject * obj = toObject(op)) {
                    Handle_t value;
                    if (!obj->Get(name, value, ctx.currentClass)) {
                        // reference error
                    }
                    ctx.operandStack[ctx.opStackPtr++] = value;
                }
                break;
            }
        case eAVM2_setproperty:
            {
                /////
                // read the name of the property to set.
                Object_MultiName name = abc->GetMultiName(io.GetU30());
                /////
                // Guard and verify the stack pointers before continuing.
                ExpectStackSize(ctx.opStackPtr, 2);

                Handle_t value      = ctx.operandStack[--ctx.opStackPtr];
                Handle_t objHandle  = ctx.operandStack[--ctx.opStackPtr]; 

                if (CScriptObject * obj = toObject(objHandle)) {
                    if (!obj->Put(name.qualifiedName, value, ctx.currentClass)) {
                        // reference error.
                    }
                } else {
                    // Not a object
                }
                break;
            }
            /**************************************************************************/
            /*                              Register access                           */
            /**************************************************************************/
        case eAVM2_getlocal:
            ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);
            ctx.operandStack[ctx.opStackPtr++] = ctx.getRegister(io.GetU30());
            break;
        case eAVM2_setlocal:
            ExpectStackSize(ctx.opStackPtr, 1);
            ctx.setRegister(io.GetU30(), ctx.operandStack[--ctx.opStackPtr]);
            break;
        case eAVM2_getlocal_0:
            /////
            // Pushes the contents of register 0 on the operand stack.
            ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);
            ctx.operandStack[ctx.opStackPtr++] = ctx.getRegister(0);
            break;
        case eAVM2_getlocal_1:
            /////
            // Pushes the contents of register 1 on the operand stack.
            ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);
            ctx.operandStack[ctx.opStackPtr++] = ctx.getRegister(1);
            break;
        case eAVM2_getlocal_2:
            /////
            // Pushes the contents of register 2 on the operand stack.
            ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);
            ctx.operandStack[ctx.opStackPtr++] = ctx.getRegister(2);
            break;
        case eAVM2_getlocal_3:
            /////
            // Pushes the contents of register 3 on the operand stack.
            ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);
            ctx.operandStack[ctx.opStackPtr++] = ctx.getRegister(3);
            break;
        case eAVM2_setlocal_0:
            ExpectStackSize(ctx.opStackPtr, 1);
            ctx.setRegister(0, ctx.operandStack[--ctx.opStackPtr]);
            break;
        case eAVM2_setlocal_1:
            ExpectStackSize(ctx.opStackPtr, 1);
            ctx.setRegister(1, ctx.operandStack[--ctx.opStackPtr]);
            break;
        case eAVM2_setlocal_2:
            ExpectStackSize(ctx.opStackPtr, 1);
            ctx.setRegister(2, ctx.operandStack[--ctx.opStackPtr]);
            break;
        case eAVM2_setlocal_3:
            ExpectStackSize(ctx.opStackPtr, 1);
            ctx.setRegister(3, ctx.operandStack[--ctx.opStackPtr]);
            break;
        case eAVM2_kill:
            /////
            // Kills a register
            assert(false);
            break;
            /**************************************************************************/
            /*                              Method invocation                         */
            /**************************************************************************/
        case eAVM2_call:
        case eAVM2_callmethod:

        case eAVM2_callproperty:
        case eAVM2_callpropvoid:
            /////
            // Call a named property on a object.
            {
                const Object_MultiName propName = abc->GetMultiName(io.GetU30());
                const size_t numArgs            = io.GetU30();
                /////
                // Make sure that the arguments and object is on stack
                ExpectStackSize(ctx.opStackPtr, numArgs + 1);
                if (CScriptObject * obj = toObject(ctx.operandStack[ctx.opStackPtr - numArgs])) {
                    Handle_t retVal;
                    err = obj->CallProperty(nullptr, propName, 
                        &ctx.operandStack[ctx.opStackPtr - numArgs], numArgs, retVal);
                    if (err == Err_Ok) {

                    } else {
                        // Failure
                    }
                } else {
                    // Not a object, so cannot have any properties
                }
                if (opcode == eAVM2_callproperty) {
                    // Push the return value onto the operand stack
                } else {
                    // Ignore the return value
                }
                break;
            }
        case eAVM2_callproplex:
        case eAVM2_callstatic:
        case eAVM2_callsuper:
        case eAVM2_callsupervoid:
            /////
            // Not implemented yet.
            assert(false);
            break;
            /**************************************************************************/
            /*                              Object construction                       */
            /**************************************************************************/
        case eAVM2_newclass:
            {
                ExpectStackSize(ctx.opStackPtr, 1);
                const abc::CAbcClassDefinition * definition = abc->GetClassDefinition(io.GetU30());
                Handle_t baseType       = ctx.operandStack[--ctx.opStackPtr];
                //CABCClassObject * cObj    = new CABCClassObject(definition, baseType);
                break;
            }
        case eAVM2_newobject:
            {
                size_t   arg_count = io.GetU30();
                /////
                // fin the class object and invoke [[Construct]] on it.
                CClassObject * classObject = virtualMachine->findClass(AVM2_OBJECT);
                if (!classObject) {
                    throw std::runtime_error("Failed to get 'Object' class instance.");
                }
                // TODO: implement
                break;
            }
        case eAVM2_construct: /**< invoke the [[Construct]] property on a object */
            {
                break;
            }
        case eAVM2_constructsuper:
            {
                Handle_t operand = ctx.operandStack[--ctx.opStackPtr];
                if (CScriptObject * obj = toObject(operand)) {
                    if (const abc::CAbcClassDefinition * superClass = ctx.currentClass) {
                        /////
                        // Execute the instance constructor of the super class
                        execute(superClass->instanceInitializer,    /**< instance constructor */
                            operand,                                /**< this object */
                            Handle_t(),                             /**< the global object */
                            0,                                      /**< timeout */
                            superClass                              /**< class */
                        );
                    } else {
                        /////
                        // Not currently executing in the context of a method.
                    }
                }
                break;
            }
        case eAVM2_constructprop:
            {
                break;
            }

            /**************************************************************************/
            /*                          Arithmetic instructions                       */
            /**************************************************************************/
            case eAVM2_add:
            {
                /////
                // Guard and verify the stack pointers before continuing.
                ExpectStackSize(ctx.opStackPtr, 2);

                Handle_t op2 = ctx.operandStack[--ctx.opStackPtr];
                Handle_t op1 = ctx.operandStack[--ctx.opStackPtr];
                ctx.operandStack[ctx.opStackPtr++] = add(op1, op2);
                break;
            }
            case eAVM2_add_i:
            {
                /////
                // Guard and verify the stack pointers before continuing.
                ExpectStackSize(ctx.opStackPtr, 2);

                Handle_t op2 = ctx.operandStack[--ctx.opStackPtr];
                Handle_t op1 = ctx.operandStack[--ctx.opStackPtr];
                ctx.operandStack[ctx.opStackPtr++] = addi(op1, op2);
                break;
            }
            case eAVM2_subtract:
            {
                /////
                // Guard and verify the stack pointers before continuing.
                ExpectStackSize(ctx.opStackPtr, 2);

                Handle_t op2 = ctx.operandStack[--ctx.opStackPtr];
                Handle_t op1 = ctx.operandStack[--ctx.opStackPtr];
                ctx.operandStack[ctx.opStackPtr++] = subtract(op1, op2);
                break;
            }
            case eAVM2_subtract_i:
            {
                /////
                // Guard and verify the stack pointers before continuing.
                ExpectStackSize(ctx.opStackPtr, 2);

                Handle_t op2 = ctx.operandStack[--ctx.opStackPtr];
                Handle_t op1 = ctx.operandStack[--ctx.opStackPtr];
                ctx.operandStack[ctx.opStackPtr++] = subtracti(op1, op2);
                break;
            }
            case eAVM2_divide:
            {
                /////
                // Guard and verify the stack pointers before continuing.
                ExpectStackSize(ctx.opStackPtr, 2);

                Handle_t op2 = ctx.operandStack[--ctx.opStackPtr];
                Handle_t op1 = ctx.operandStack[--ctx.opStackPtr];
                ctx.operandStack[ctx.opStackPtr++] = divide(op1, op2);
                break;
            }
            /**************************************************************************/
            /*                                  Constants                             */
            /**************************************************************************/
            case eAVM2_pushdouble:
                /////
                // Push a double value onto the operand stack
                ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);
                ctx.operandStack[ctx.opStackPtr++] = numberToAtom(pool.GetDouble(io.GetU30()));
                break;
            case eAVM2_pushfalse:
                /////
                // Push 'false' onto the operand stack
                ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);
                ctx.operandStack[ctx.opStackPtr++] = False_atom;
                break;
            case eAVM2_pushtrue:
                /////
                // Push 'true' onto the operand stack
                ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);
                ctx.operandStack[ctx.opStackPtr++] = True_atom;
                break;
            case eAVM2_pushint:
                /////
                // Push a U32 value onto the stack
                ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);
                ctx.operandStack[ctx.opStackPtr++] = u32ToAtom(pool.GetU32(io.GetU30()));
                break;
            case eAVM2_pushuint:
                /////
                // Push a S32 value onto the stack.
                ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);
                ctx.operandStack[ctx.opStackPtr++] = s32ToAtom(pool.GetS32(io.GetU30()));
                break;
            case eAVM2_pushnan:
                /////
                // Push 'NaN' onto the stack
                ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);
                ctx.operandStack[ctx.opStackPtr++] = Nan_atom;
                break;
            case eAVM2_pushnull:
                /////
                // Push 'NULL' onto the stack
                ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);
                ctx.operandStack[ctx.opStackPtr++] = Null_atom;
                break;
            case eAVM2_pushundefined:
                /////
                // Push 'undefined' onto the stack
                ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);
                ctx.operandStack[ctx.opStackPtr++] = Undefined_atom;
                break;
            case eAVM2_pushbyte:
                /////
                // Push a 8 bit integer onto the stack.
                ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);
                ctx.operandStack[ctx.opStackPtr++] = s32ToAtom((int32_t) ((char)io.GetU8()));
                break;
            case eAVM2_pushshort:
                /////
                //
                assert(false);
                break;
            case eAVM2_pushstring:
                /////
                //
                assert(false);
                break;
            case eAVM2_pop:
                /////
                // Pop the top of the operand stack
                
                /////
                // Make sure that there is at least on item on the stack.
                ExpectStackSize(ctx.opStackPtr, 1);
                --ctx.opStackPtr;
                break;
            case eAVM2_dup:
                /////
                // Duplicate to top most item on the operand stack.
                ExpectStackSize(ctx.opStackPtr, 1);
                ExpectNotFull(ctx.maxOperandStack, ctx.opStackPtr);
                ctx.operandStack[ctx.opStackPtr++] = ctx.operandStack[ctx.opStackPtr - 1];
                break;
            case eAVM2_swap:
                /////
                // Swap the two top most item on the stack
                ExpectStackSize(ctx.opStackPtr, 2);
                std::swap(ctx.operandStack[ctx.opStackPtr-1], ctx.operandStack[ctx.opStackPtr-2]);
                break;
            default:
                break;
        }
    }

    return Err_Ok;
}

Handle_t CInterpreter::add(Handle_t _x, Handle_t _y)
{
    return Undefined_atom;
}

Handle_t CInterpreter::addi(Handle_t _x, Handle_t _y)
{
    return Undefined_atom;
}

Handle_t CInterpreter::subtract(Handle_t _x, Handle_t _y)
{
    return Undefined_atom;
}

Handle_t CInterpreter::subtracti(Handle_t _x, Handle_t _y)
{
    return Undefined_atom;
}

Handle_t CInterpreter::divide(Handle_t _x, Handle_t _y)
{
    return Undefined_atom;
}

}