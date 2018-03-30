#ifndef _CABCCLASSDEFINITION_H_
#define _CABCCLASSDEFINITION_H_

/**
 * \file    CAbcClassDefinition.h
 * \brief   Represents a class that was defined in the ABC binary.
 * \author  Magnus Leksell
 */
#include "Names.h"
#include <map>
#include "abctypes.h"

namespace avm2
{

namespace abc
{

class CAbcMethodDefinition;

struct TraitInfo
{
    uint32_t trait_type     : 4;    /**< defines what kind of trait it is */
    uint32_t class_index    : 12;   /**< defines what class defined the trait */
    uint32_t index          : 16;   /**< index into value array, or index into methods */
};

/**
 * Representation of a class defined in the ABC.
 */
class CAbcClassDefinition
{
public:
    CAbcMethodDefinition *          staticInitializer;      /**< static constructor */
    CAbcMethodDefinition *          instanceInitializer;    /**< instance constructor */
    CAbcClassDefinition *           superClass;             /**< super class */
    QName                           baseType;
    std::vector<TraitInfo>          traits;                 /**< defines the traits */
    std::vector<QName>              traitNames;             /**< the name of the traits */
    size_t                          numSlots;
    size_t                          abcIndex;               /**< the ABC index for the class */
};

} // namespace abc

} // namespace avm2

#endif