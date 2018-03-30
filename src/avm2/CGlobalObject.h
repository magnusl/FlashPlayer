#ifndef _CGLOBALOBJECT_H_
#define _CGLOBALOBJECT_H_

#include "CScriptObject.h"

namespace avm2
{

/**
 * Extends the CScriptObject with the capability of registering
 * packages that will be initialized on demand.
 */
class CGlobalObject : public CScriptObject
{
public:
    explicit CGlobalObject(CScriptObject *);

};

}

#endif