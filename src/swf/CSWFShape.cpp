#include "CSWFShape.h"
#include "CDisplayObject.h"
#include "swfModule.h"
#include <map>

using namespace std;
using namespace avm2;

namespace swf
{

CSWFShape_Class::CSWFShape_Class(avm2::CClassObject * cObject, 
        avm2::vtable_t * vt,
        avm2::CVirtualMachine & vm, 
        CClassObject * baseType) :
CClassObject(cObject, vt, vm, baseType)
{
    name = QName(vm.getStringTable().set("Shape"));
}

avm2::Handle_t CSWFShape_Class::construct(const std::vector<avm2::Handle_t> & arguments)
{
    return nullptr;
}

uint32_t CSWFShape_Class::getId() const
{
    return SWF_EMBEDDEDSHAPE;
}

/////
// vtable for static SWFShape properties
DEFINE_VTABLE(CSWFShape_Class, CClassObject)
END_VTABLE()

/*****************************************************************************/
/*                                  Instance                                 */
/*****************************************************************************/
CSWFShape::CSWFShape(avm2::CClassObject * cObject, avm2::vtable_t * vt) : 
    CDisplayObject(cObject, vt)
{
}

///////
// CDisplayObject
void CSWFShape::setShape(std::shared_ptr<gfx::Shape_t> shape)
{
    swfShape = shape;
}

void CSWFShape::draw(IRenderer & renderer)
{
    if (hasColorTransform) {
        renderer.pushColorTransform(colorTransform);
    }

    // draw the shape
    Eigen::Matrix3f global;
    getGlobalTransformation(global);
    renderer.draw(swfShape, global, this->alpha);

    if (hasColorTransform) {
        renderer.popColorTransform();
    }
}

void CSWFShape::advanceTimeline(swf::CPlayer & player)
{
    // empty
}

/////
// Exported properties
DEFINE_VTABLE(CSWFShape, CDisplayObject)
END_VTABLE()

#if 0
/*****************************************************************************/
/*                          Internal implementation                          */
/*****************************************************************************/
struct ShapeComponent_t {
    std::shared_ptr<gfx::Shape_t> sharedShape;
};

class CSWFShape_impl {
public:
    ShapeComponent_t * getComponent(size_t);
    bool addComponent(size_t id);
    bool removeComponent(size_t id);

    CDisplayObject *                displayClass;
    map<uint32_t, ShapeComponent_t> objects;
};

/**
 * Returns the component with is associated to the specified id, 
 * or NULL if no such component exists.
 */
ShapeComponent_t * CSWFShape_impl::getComponent(size_t id)
{
    map<uint32_t, ShapeComponent_t>::iterator it = objects.find(id);
    if (it == objects.end()) {
        return 0;
    }
    return &it->second;
}

bool CSWFShape_impl::addComponent(size_t id)
{
    ShapeComponent_t comp;
    objects[id] = comp;
    return true;
}

bool CSWFShape_impl::removeComponent(size_t id)
{
    map<uint32_t, ShapeComponent_t>::iterator it = objects.find(id);
    if (it != objects.end()) {
        objects.erase(it);
        return true;
    }
    return false;
}

/*****************************************************************************/
/*                                  CSWFShape                                */
/*****************************************************************************/

CSWFShape::CSWFShape(CClassObject * baseClass, avm2::CVirtualMachine & vm) :
    CDisplayClass(baseClass, vm)
{
    impl = new CSWFShape_impl();
    impl->displayClass = (swf::CDisplayObject *)vm.getClass(SWF_DISPLAYOBJECT);
}

CSWFShape::~CSWFShape()
{
    delete impl;
}

bool CSWFShape::initObject(avm2::Object_t * obj)
{
    if (!obj) {
        return false;
    }
    if (!impl->addComponent(obj->id)) {
        return false;
    }
    return baseClass->initObject(obj);
}

bool CSWFShape::destroyObject(avm2::Object_t * obj)
{
    if (!impl->removeComponent(obj->id)) {
        return false;
    }
    return baseClass->destroyObject(obj);
}

bool CSWFShape::setShape(avm2::Object_t * obj, std::shared_ptr<gfx::Shape_t> gfxShape)
{
    if (!obj) {
        return false;
    }
    if (ShapeComponent_t * shape = impl->getComponent(obj->id)) {
        shape->sharedShape = gfxShape;
    } else {
        return false;
    }
    return true;
}

/**
 * Draws a SWF shape.
 */
void CSWFShape::draw(IRenderer * renderer, avm2::Object_t * obj)
{
    CDisplayObject::IDisplayComponent * 
        displayObject = impl->displayClass->GetComponent(obj);
    if (!displayObject) {
        throw std::runtime_error("Object is not a display object.");
    }
    if (ShapeComponent_t * shape = impl->getComponent(obj->id)) {
        if (displayObject->hasColorTransform()) {
            renderer->pushColorTransform(displayObject->getColorTransform());
        }
        Eigen::Matrix3f global;
        displayObject->getGlobalTransform(global);
        renderer->draw(shape->sharedShape, global, displayObject->getAlpha());
        if (displayObject->hasColorTransform()) {
            renderer->popColorTransform();
        }
    } else {
        throw std::runtime_error("Object is not a SWF shape.");
    }
}

void CSWFShape::useAsClipMask(IRenderer * renderer, avm2::Object_t * obj)
{
    CDisplayObject::IDisplayComponent * 
        displayObject = impl->displayClass->GetComponent(obj);
    if (!displayObject) {
        throw std::runtime_error("Object is not a display object.");
    }
    if (ShapeComponent_t * shape = impl->getComponent(obj->id)) {
        Eigen::Matrix3f global;
        displayObject->getGlobalTransform(global);
        renderer->pushClipMask(shape->sharedShape, global);
    }
}

bool CSWFShape::hitTestPoint(avm2::Object_t * obj, 
                             const swf::gfx::Point_t<float> & point, 
                             bool shapeFlag)
{
    return false;
}

bool CSWFShape::advanceTimeline(CPlayer & player, avm2::Object_t *)
{
    return true;
}

#endif

}