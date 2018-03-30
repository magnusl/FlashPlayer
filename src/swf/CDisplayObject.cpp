#include "CDisplayObject.h"
#include "swfModule.h"
#include <map>

using namespace avm2;
using namespace std;

namespace swf
{
/*****************************************************************************/
/*                                  Class                                    */
/*****************************************************************************/
CDisplayObject_Class::CDisplayObject_Class(CClassObject * cObject, avm2::vtable_t * vt,
        CVirtualMachine & vm, CClassObject * baseType) : 
    CClassObject(cObject, vt, vm, baseType)
{
    name = QName(vm.getStringTable().set("DisplayObject"));
}

/**
 * Constructs a flash.display.DisplayObject instance.
 */
Handle_t CDisplayObject_Class::construct(const std::vector<Handle_t> & arguments)
{
    CDisplayObject * obj = new CDisplayObject(this, &CDisplayObject::vtable);
    return obj;
}

uint32_t CDisplayObject_Class::getId() const
{
    return SWF_DISPLAYOBJECT;
}

/*****************************************************************************/
/*                                  Instance                                 */
/*****************************************************************************/
CDisplayObject::CDisplayObject(CClassObject * cObject, vtable_t * vt) :
    CEventDispatcher(cObject, vt),
    parent(0),
    rotation(0.0f),
    alpha(1.0f),
    name(0),
    clipDepth(0),
    visible(1),
    dirtyTransform(1),
    hasColorTransform(0)
{
    scaleFactor.X = scaleFactor.Y = 1.0f;
}

CDisplayObject::~CDisplayObject()
{
}

void CDisplayObject::setRotation(float _angle)
{
    rotation        = _angle;
    dirtyTransform  = 1;
}

void CDisplayObject::setPosition(float _x, float _y)
{
    position.X      = _x;
    position.Y      = _y;
    dirtyTransform  = 1;
}

void CDisplayObject::setScale(float _sx, float _sy)
{
    scaleFactor.X   = _sx;
    scaleFactor.Y   = _sy;
    dirtyTransform  = 1;
}

void CDisplayObject::setParent(CDisplayObject * _parent)
{
    parent = _parent;
}

void CDisplayObject::setName(avm2::CStringTable::index_t _name)
{
    name = _name;
}

void CDisplayObject::setColorTransform(const gfx::ColorTransform & cx)
{
    colorTransform = cx;
}

const Eigen::Matrix3f & CDisplayObject::getLocalTransformation()
{
    if (dirtyTransform) {
        localTransform = Eigen::Matrix3f::Identity();
        // rotation component
        if (rotation) {
            Eigen::Matrix3f rotMat = Eigen::AngleAxis<float>(
                (rotation * 0.0174532925f), Eigen::Vector3f(0, 0, -1)).toRotationMatrix();
            localTransform *= rotMat;
        }
        // position
        localTransform.col(2) = Eigen::Vector3f(position.X, position.Y, 1.0f);
        // scaling
        Eigen::Matrix3f scaleMat = Eigen::Scaling(scaleFactor.X, scaleFactor.Y, 1.0f);
        localTransform *= scaleMat;
    }
    return localTransform;
}

void CDisplayObject::setVisible(bool)
{
    visible = 1;
}

gfx::Point_t<float> CDisplayObject::globalToLocal(const gfx::Point_t<float> & _point)
{
    return _point;
}

gfx::Point_t<float> CDisplayObject::localToGlobal(const gfx::Point_t<float> & _point)
{
    return _point;
}

void CDisplayObject::getGlobalTransformation(Eigen::Matrix3f & mat)
{
    if (!parent) {
        mat = getLocalTransformation();
    } else {
        Eigen::Matrix3f global;
        parent->getGlobalTransformation(global);
        mat = global * getLocalTransformation();
    }
}

void CDisplayObject::setClipDepth(uint16_t depth)
{
    clipDepth = depth;
}

uint16_t CDisplayObject::getClipDepth() const
{
    return clipDepth;
}

bool CDisplayObject::hitTestPoint(const gfx::Point_t<float> &, bool pixelPerfect)
{
    return false;
}

void CDisplayObject::draw(IRenderer &)
{
    // empty
}

void CDisplayObject::advanceTimeline(swf::CPlayer &)
{
    // empty
}

/////
// GC
void CDisplayObject::markReachable()
{
    if (reachable) { /**< already visited */
        return;
    }
    reachable = 1;
    if (parent) {
        parent->markReachable();
    }
    // make sure to mark any listeners as reachable
    CEventDispatcher::markReachable();
}

/**
 * Define the vtable that actually exports the methods to the script.
 */
DEFINE_VTABLE(CDisplayObject, CEventDispatcher)
    EXPORT_PROPERTY("alpha", &CDisplayObject::getter_alpha, nullptr)
    EXPORT_PROPERTY("height", &CDisplayObject::getter_height, nullptr)
    EXPORT_PROPERTY("name", &CDisplayObject::getter_name, nullptr)
    EXPORT_PROPERTY("parent", &CDisplayObject::getter_parent, nullptr)
    EXPORT_PROPERTY("root", &CDisplayObject::getter_root, nullptr)
    EXPORT_PROPERTY("rotation", &CDisplayObject::getter_rotation, nullptr)
    EXPORT_PROPERTY("scaleX", &CDisplayObject::getter_scaleX, nullptr)
    EXPORT_PROPERTY("scaleY", &CDisplayObject::getter_scaleY, nullptr)
    EXPORT_PROPERTY("stage", &CDisplayObject::getter_stage, nullptr)
    EXPORT_PROPERTY("visible", &CDisplayObject::getter_visible, nullptr)
    EXPORT_PROPERTY("width", &CDisplayObject::getter_width, nullptr)
    EXPORT_PROPERTY("x", &CDisplayObject::getter_x, nullptr)
    EXPORT_PROPERTY("y", &CDisplayObject::getter_y, nullptr)
END_VTABLE()

avm2::Handle_t CDisplayObject::getter_alpha(void)
{
    return numberToAtom(this->alpha);
}

avm2::Handle_t CDisplayObject::getter_height(void)
{
    return Handle_t();
}

avm2::Handle_t CDisplayObject::getter_name(void)
{
    return Handle_t();
}

avm2::Handle_t CDisplayObject::getter_parent(void)
{
    if (parent) {
        return objToAtom(parent);
    } else {
        return Null_atom;
    }
}

avm2::Handle_t CDisplayObject::getter_root(void)
{
    return Handle_t();
}

avm2::Handle_t CDisplayObject::getter_rotation(void)
{
    return Handle_t();
}

avm2::Handle_t CDisplayObject::getter_scaleX(void)
{
    return numberToAtom(scaleFactor.X);
}

avm2::Handle_t CDisplayObject::getter_scaleY(void)
{
    return numberToAtom(scaleFactor.Y);
}

avm2::Handle_t CDisplayObject::getter_stage(void)
{
    return Handle_t();
}

avm2::Handle_t CDisplayObject::getter_visible(void)
{
    return Handle_t();
}

avm2::Handle_t CDisplayObject::getter_width(void)
{
    return Handle_t();
}

avm2::Handle_t CDisplayObject::getter_x(void)
{
    return numberToAtom(position.X);
}

avm2::Handle_t CDisplayObject::getter_y(void)
{
    return numberToAtom(position.Y);
}

}