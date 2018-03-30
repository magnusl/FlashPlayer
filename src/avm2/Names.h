#ifndef _NAMES_H_
#define _NAMES_H_

/**
 * \file    Names.h
 * \brief   Defines the various name types available in the ActionScript runtime.
 * \author  Magnus Leksell
 */

#include <stdint.h>
#include <vector>

namespace avm2
{

/**
 * Qualified Name, a name and a namespace.
 */
struct QName
{
    uint16_t Name;
    uint16_t Namespace;
    
    QName()
    {
    }

    QName(uint16_t name, uint16_t ns = 0) : 
        Name(name),
        Namespace(ns)
    {
    }

    bool operator==(const QName & _name) const
    {
        return (_name.Name == Name) && (_name.Namespace == Namespace);
    }

    uint32_t toU32() const
    {
        return ((uint32_t) Namespace) << 16 | Name;
    }
};

/** Multiname entries usually consist of a name index, and a namespace or 
    namespace set index */
struct Object_MultiName
{
    enum {
        eQualifiedName,
        eRuntimeQualified,
        eRuntimeQualifiedLate,
        eMultiname,
        MultipleNamespaceLate
    } Type;

    QName qualifiedName;
    std::vector<uint16_t> namespaces;
    Object_MultiName()
    {
    }

    Object_MultiName(const QName & a_Name)
    {
        Type = eQualifiedName;
        qualifiedName = a_Name;
    }

    bool isRuntime() const
    {
        return false;
    }

    void operator=(const QName & a_Name)
    {
        Type = eQualifiedName;
        qualifiedName = a_Name;
    }

    bool operator==(const QName & a_Name) const
    {
        if (Type == eQualifiedName) {
            return (qualifiedName.Name == a_Name.Name) && 
                (qualifiedName.Namespace == a_Name.Namespace);
        } else if (Type == eMultiname) {
            for(size_t i = 0, count = namespaces.size(); i < count; ++i) {
                if (namespaces[i] == a_Name.Namespace) {
                    return (a_Name.Name == qualifiedName.Name);
                }
            }
        }
        return false;
    }
};

} // namespace avm2

#endif
