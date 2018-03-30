#ifndef _COBJECTCLASS_H_
#define _COBJECTCLASS_H_

#include "CClassObject.h"

namespace avm2
{

/**
 * Class object for the 'Object' class
 */
class CObject_Class : public CClassObject
{
public:
    CObject_Class(avm2::CClassObject * cObject, 
        avm2::vtable_t * vt,
        avm2::CVirtualMachine & vm);

    /**
     * Constructs an instance of the class.
     *
     * \param [in] arguments    The arguments to supply to the class constructor.
     */
    virtual avm2::Handle_t construct(const std::vector<avm2::Handle_t> & arguments);
    virtual uint32_t getId() const;

    DECLARE_VTABLE()
};

}
#endif
