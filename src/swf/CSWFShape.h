#ifndef _CSWFSHAPE_H_
#define _CSWFSHAPE_H_

#include "CDisplayObject.h"
#include "gfxShape.h"
#include <memory>
#include "swfModule.h"
#include <avm2\CABCClassObject.h>

namespace swf
{

/**
 * flash.display.Shape
 */
class CSWFShape : public CDisplayObject
{
    DECLARE_DERIVED(SWF_EMBEDDEDSHAPE, CDisplayObject)
public:
    CSWFShape(avm2::CClassObject * cObject, avm2::vtable_t * vt);
    void setShape(std::shared_ptr<gfx::Shape_t>);

    ///////
    // CDisplayObject
    virtual bool hitTestPoint(const gfx::Point_t<float> &, bool pixelPerfect = false) { return false; }
    virtual void draw(IRenderer &);
    virtual void advanceTimeline(swf::CPlayer &);

    /////
    // exported properties
    avm2::Handle_t  getter_graphics(void);

private:
    std::shared_ptr<gfx::Shape_t> swfShape;

    DECLARE_VTABLE()
};

class CSWFShape_Class : public avm2::CClassObject
{
public:
    CSWFShape_Class(avm2::CClassObject * cObject, 
        avm2::vtable_t * vt,
        avm2::CVirtualMachine & vm, 
        CClassObject * baseType);
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
