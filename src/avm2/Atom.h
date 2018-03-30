#ifndef _ATOM_H_
#define _ATOM_H_

#include <stdint.h>

namespace avm2
{
    class CScriptObject;

    typedef uint64_t Atom_t;

    typedef enum {
        eUNDEFINED  = 0,    /**< undefined */
        eOBJECT     = 1,    /**< identifies a object */
        eS32        = 2,    /**< 32 bit signed integer */
        eU32        = 3,    /**< 32 bit signed integer */
        eNUMBER     = 4,
        eSTRING     = 5,
        eNAN        = 6,
        eBOOL       = 7
    } AtomType_t;

    inline AtomType_t getType(Atom_t _value)
    {
        return static_cast<AtomType_t>(_value & 0x07);
    }

    inline CScriptObject * toObject(Atom_t _value)
    {
        if (getType(_value) == eOBJECT) {
            return reinterpret_cast<CScriptObject *>(_value & ~0x07);
        }
        return 0;
    }

    inline Atom_t u32ToAtom(uint32_t value)
    {
        return (((Atom_t)value) << 3) | eU32;
    }

    inline Atom_t s32ToAtom(int32_t _value)
    {
        return (((Atom_t)_value) << 3) | eS32;
    }

    inline Atom_t numberToAtom(double _value)
    {
        return (*((uint64_t *) &_value) & 0xfffffffffffffff8) | eNUMBER;
    }

    inline Atom_t objToAtom(CScriptObject * obj)
    {
        return (((uint64_t) obj) & 0xfffffffffffffff8) | eOBJECT;
    }

#define Undefined_atom  ((avm2::Atom_t) avm2::eUNDEFINED)
#define Null_atom ((avm2::Atom_t) avm2::eOBJECT)
#define Nan_atom ((avm2::Atom_t) avm2::eNAN)
#define True_atom ((avm2::Atom_t) ((1 << 3) | avm2::eBOOL))
#define False_atom ((avm2::Atom_t) avm2::eBOOL)

}

#endif