#ifndef _CORETYPES_H_
#define _CORETYPES_H_

namespace avm2
{

#define AVM2_MODULE_ID (0 << 24)

enum {
    AVM2_Object,
    AVM2_Array,
    AVM2_String,
    AVM2_ClassObject,
    AVM2_FunctionObject,
    AVM2_AbcClassObject,

    // this must always be the last one.
    AVM2_MaxClass
};

enum 
{
    AVM2_OBJECT         = (AVM2_MODULE_ID | AVM2_Object), 
    AVM2_ARRAY          = (AVM2_MODULE_ID | AVM2_Array),
    AVM2_STRING         = (AVM2_MODULE_ID | AVM2_String),
    AVM2_CLASSOBJECT    = (AVM2_MODULE_ID | AVM2_ClassObject),
    AVM2_ABCCLASSOBJECT = (AVM2_MODULE_ID | AVM2_AbcClassObject)
};

}

#endif