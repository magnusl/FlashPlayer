#ifndef _CDISPLAYOBJECT_H_
#define _CDISPLAYOBJECT_H_

#include "CEventDispatcher.h"
#include <Eigen\Dense>
#include "gfxPoint.h"
#include "IRenderer.h"
#include "CPlayer.h"
#include <avm2\CStringTable.h>
#include "swfModule.h"
#include <avm2\CClassObject.h>

namespace swf
{

/**
 * flash.display.DisplayObject instance
 */
class CDisplayObject : public CEventDispatcher
{
    DECLARE_DERIVED(SWF_DISPLAYOBJECT, CEventDispatcher)

public:
    CDisplayObject(avm2::CClassObject * _constructorObject, avm2::vtable_t * vt);

    virtual ~CDisplayObject();

    void setRotation(float);
    void setPosition(float, float);
    void setScale(float, float);
    const Eigen::Matrix3f & getLocalTransformation();
    void setVisible(bool);
    gfx::Point_t<float> globalToLocal(const gfx::Point_t<float> &);
    gfx::Point_t<float> localToGlobal(const gfx::Point_t<float> &);
    void getGlobalTransformation(Eigen::Matrix3f &);
    void setClipDepth(uint16_t);
    uint16_t getClipDepth() const;
    void setName(avm2::CStringTable::index_t);
    avm2::CStringTable::index_t getName() const;
    void setColorTransform(const gfx::ColorTransform &);
    void setParent(CDisplayObject *);

    // Methods that may need to implemented in a subclass.
    virtual bool hitTestPoint(const gfx::Point_t<float> &, bool pixelPerfect = false);
    virtual void draw(IRenderer &);
    virtual void advanceTimeline(swf::CPlayer &);

    /////
    // GC
    virtual void markReachable();

    /////
    // Property access
    avm2::Handle_t getter_alpha(void);
    avm2::Handle_t getter_height(void);
    avm2::Handle_t getter_name(void);
    avm2::Handle_t getter_parent(void);
    avm2::Handle_t getter_root(void);
    avm2::Handle_t getter_rotation(void);
    avm2::Handle_t getter_scaleX(void);
    avm2::Handle_t getter_scaleY(void);
    avm2::Handle_t getter_stage(void);
    avm2::Handle_t getter_visible(void);
    avm2::Handle_t getter_width(void);
    avm2::Handle_t getter_x(void);
    avm2::Handle_t getter_y(void);

protected:
    Eigen::Matrix3f             localTransform; /**< the local transformation */
    gfx::ColorTransform         colorTransform;
    gfx::Point_t<float>         position;       /**< position relative parent */
    gfx::Point_t<float>         scaleFactor;    /**< scale factor */
    CDisplayObject *            parent;
    float                       rotation;       /**< rotation angle */
    float                       alpha;          /**< alpha component */
    avm2::CStringTable::index_t name;           /**< the name of the object */
    uint16_t                    clipDepth;

    uint8_t visible             : 1;        /**< indicates if the object is visible */
    uint8_t dirtyTransform      : 1;        /**< dirty transformation matrix */
    uint8_t hasColorTransform   : 1;        /**< indicates if the object has a cxform */

    DECLARE_VTABLE()
};

/**
 * flash.display.DisplayObject class object.
 */
class CDisplayObject_Class : public avm2::CClassObject
{
public:
    CDisplayObject_Class(avm2::CClassObject * cObject, 
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
};

}

#endif