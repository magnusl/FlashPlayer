#ifndef _ABCTYPES_H_
#define _ABCTYPES_H_

#include <stdint.h>
#include <vector>
#include "Names.h"

/**
 * \file    abctypes.h
 * \brief   Defines the various types used in the ABC delivery.
 * \author  Magnus Leksell
 */
namespace avm2
{
namespace abc
{
    enum {
        CONSTANT_QName          = 0x07,
        CONSTANT_QNameA         = 0x0D,
        CONSTANT_RTQName        = 0x0F,
        CONSTANT_RTQNameA       = 0x10,
        CONSTANT_RTQNameL       = 0x11,
        CONSTANT_RTQNameLA      = 0x12,
        CONSTANT_Multiname      = 0x09,
        CONSTANT_MultinameA     = 0x0E,
        CONSTANT_MultinameL     = 0x1B,
        CONSTANT_MultinameLA    = 0x1C
    };

    typedef enum {
        Trait_Slot = 0,
        Trait_Method = 1,
        Trait_Getter = 2,
        Trait_Setter = 3,
        Trait_Class = 4,
        Trait_Function = 5,
        Trait_Const = 6,
    } TraitType_t;

    struct traits_info
    {
        QName       name;
        uint8_t     kind;
        uint8_t     attr;

        enum {
            ATTR_Final      = 0x01,
            ATTR_Override   = 0x02,
            ATTR_Metadata   = 0x04,
        };

        union {
            struct {
                uint32_t slotid;
                uint32_t type_name;
                uint32_t vindex;
                uint8_t  vkind;
            } trait_slot;

            struct {
                uint32_t slotid;
                uint32_t classi;
            } trait_class;

            struct {
                uint32_t slotid;
                uint32_t function;
            } trait_function;

            struct {
                uint32_t dispid;
                uint32_t method;
            } method;
        } u;
    };

    struct namespace_info
    {
        uint8_t     kind;
        uint32_t    name;
    };

    struct ns_set_info
    {
        std::vector<uint32_t> ns;
    };

    struct MultiName {
        uint8_t kind;
        union {
            struct {
                uint32_t ns;
                uint32_t name;
            } QName;
            struct {
                uint32_t name;
            } RTQName;
            struct {
                uint32_t name;
                uint32_t ns_set;
            } Multiname;
            struct {
                uint32_t ns_set;
            } MultinameL;
        } u;
    };

    struct option_detail
    {
        uint32_t    val;
        uint8_t     kind;
    };

    struct option_info 
    {
        std::vector<option_detail>  option;
    };

    struct method_info
    {
        uint32_t                param_count;
        uint32_t                return_type;
        std::vector<uint32_t>   param_type;
        uint32_t                name;
        uint8_t                 flags;
        option_info             options;
        std::vector<uint32_t>   param_names;
    };

    struct instance_info
    {
        uint32_t                    name;
        uint32_t                    super_name;
        uint8_t                     flags;
        uint32_t                    protectedNs;
        std::vector<uint32_t>       interfaces;
        uint32_t                    iinit;
        std::vector<traits_info>    traits;
    };

    struct class_info
    {
        uint32_t                    cinit;
        std::vector<traits_info>    traits;
    };

    struct script_info
    {
        uint32_t                    init;
        std::vector<traits_info>    traits;
    };

    struct exception_info
    {
        uint32_t from;
        uint32_t to; 
        uint32_t target; 
        uint32_t exc_type;
        uint32_t var_name;
    };

    struct method_body_info
    {
        uint32_t method;
        uint32_t max_stack;
        uint32_t local_count;
        uint32_t init_scope_depth;
        uint32_t max_scope_depth;
        std::vector<exception_info> exceptions;
        std::vector<uint8_t> code;
        std::vector<traits_info>    traits;
    };

} // namespace abc

} // namespace avm2

#endif