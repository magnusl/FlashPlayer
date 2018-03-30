#include "CSprite.h"
#include "swfAction.h"
#include "CDisplayObjectContainer.h"
#include <map>

using namespace std;
using namespace avm2;

namespace swf
{

/*****************************************************************************/
/*                                  Class                                    */
/*****************************************************************************/
CSprite_Class::CSprite_Class(CClassObject * cObject, 
                             avm2::vtable_t * vt,
                             CVirtualMachine & vm, 
                             CClassObject * baseType) : 
    CClassObject(cObject, vt, vm, baseType)
{
    name = QName(vm.getStringTable().set("Sprite"));
}

avm2::Handle_t CSprite_Class::construct(const std::vector<avm2::Handle_t> & arguments)
{
    return nullptr;
}

uint32_t CSprite_Class::getId() const
{
    return SWF_SPRITE;
}

/*****************************************************************************/
/*                                  Instance                                 */
/*****************************************************************************/
CSprite::CSprite(avm2::CClassObject * cObject, vtable_t * vt) : 
    CDisplayObjectContainer(cObject, vt)
{
}

DEFINE_VTABLE(CSprite, CDisplayObjectContainer)
END_VTABLE()

#if 0
struct SpriteComponent_t
{
    std::shared_ptr<swf::MovieClip> frames;
};

class CSprite_Impl
{
public:
    std::map<uint32_t, SpriteComponent_t> objects;
};

CSprite::CSprite(CClassObject * baseClass, avm2::CVirtualMachine & vm) :
    CDisplayClass(baseClass, vm)
{
    impl = new (std::nothrow) CSprite_Impl();
}

CSprite::~CSprite()
{
    delete impl;
}

bool CSprite::initObject(avm2::Object_t * obj)
{
    if (impl && obj) {
        map<uint32_t, SpriteComponent_t>::iterator it = impl->objects.find(obj->id);
        if (it != impl->objects.end()) {
            return false;
        }
        SpriteComponent_t sprite;
        impl->objects[obj->id] = sprite;
        return baseClass->initObject(obj);
    }
    return false;
}

bool CSprite::destroyObject(avm2::Object_t * obj)
{
    if (impl && obj) {
        map<uint32_t, SpriteComponent_t>::iterator it = impl->objects.find(obj->id);
        if (it == impl->objects.end()) {
            return false;
        }
        impl->objects.erase(it);
        return baseClass->destroyObject(obj);
    }
    return false;
}

bool CSprite::advanceTimeline(CPlayer & player, avm2::Object_t * obj)
{

    if (baseClass) {
        return ((CDisplayClass *) baseClass)->advanceTimeline(player, obj);
    }
    return false;
}

void CSprite::draw(IRenderer * renderer, avm2::Object_t * obj)
{
    if (CDisplayClass * base = (CDisplayClass*) baseClass) {
        base->draw(renderer, obj);
    }
}

bool CSprite::setFrames(avm2::Object_t * obj, std::shared_ptr<swf::MovieClip> frames)
{
    if (impl && obj) {
        map<uint32_t, SpriteComponent_t>::iterator it = impl->objects.find(obj->id);
        if (it == impl->objects.end()) {
            return false;
        }
        it->second.frames = frames;
        return true;
    }
    return false;
}

std::shared_ptr<swf::MovieClip> CSprite::getFrames(avm2::Object_t * obj)
{
    if (impl && obj) {
        map<uint32_t, SpriteComponent_t>::iterator it = impl->objects.find(obj->id);
        if (it != impl->objects.end()) {
            return it->second.frames;
        }
    }
    return std::shared_ptr<swf::MovieClip>();
}

#endif
}