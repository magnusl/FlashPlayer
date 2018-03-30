#ifndef _OBJECTDEFS_H_
#define _OBJECTDEFS_H_

#include "Atom.h"

#define DECLARE_SUPER(type) \
    public: \
    enum {Type = type}; \
    virtual bool Is(size_t a_Type) {return (a_Type == Type);}

#define DECLARE_DERIVED(type, superclass) \
    public: \
    enum {Type = type}; \
    virtual bool Is(size_t a_Type) {return (a_Type == Type) ? true : superclass::Is(a_Type);}

namespace avm2
{

typedef uint32_t ClassID_t;
typedef uint32_t PackageID_t;

inline uint32_t GetPackageID(ClassID_t _id)
{
    // the package ID is stored in the upper 8 bits.
    return _id >> 24;
}

template<class T>
T * cast(Atom_t atom)
{
    // is this a object
    CScriptObject * obj = toObject(atom);
    if (obj) {
        return obj->Is(T::Type) ? ((T *) obj) : ((T *) 0);
    }
    return 0;
}

}
#endif