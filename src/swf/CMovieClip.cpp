#include "CMovieClip.h"
#include <map>
#include "CSprite.h"
#include "CPlayer.h"
#include "CDisplayObjectContainer.h"
#include "swfModule.h"
#include <iostream>

using namespace std;
using namespace avm2;

namespace swf
{

/*****************************************************************************/
/*                                  Class                                    */
/*****************************************************************************/
CMovieClip_Class::CMovieClip_Class(CClassObject * cObject, 
                                   avm2::vtable_t * vt,
                                   CVirtualMachine & vm,
                                   CClassObject * baseType) : 
CClassObject(cObject, vt, vm, baseType)
{
    name = QName(vm.getStringTable().set("MovieClip"));
}

Handle_t CMovieClip_Class::construct(const std::vector<Handle_t> & arguments)
{
    return nullptr;
}

uint32_t CMovieClip_Class::getId() const
{
    return SWF_MOVIECLIP;
}


/*****************************************************************************/
/*                                  Instance                                 */
/*****************************************************************************/
CMovieClip::CMovieClip(avm2::CClassObject * cObject, vtable_t * vt) :
    CSprite(cObject, vt),
    currentFrame(-1)
{
}

CMovieClip::~CMovieClip()
{
}

bool CMovieClip::isDone() const
{
    if (frames) {
        return currentFrame >= (int32_t) frames->frames.size();
    }
    return false;
}

void CMovieClip::resetTimeline()
{
    removeChildren();
    currentFrame = -1;
}

void CMovieClip::advanceTimeline(swf::CPlayer & player)
{
    if (currentFrame == -1) {
        currentFrame = 0;
    } else {
        ++currentFrame;
    }
    if (frames) {
        if (currentFrame < static_cast<int32_t>(frames->frames.size())) {
            player.ExecuteFrame(frames->frames[currentFrame], this);
        }
    }
    // make sure that we advance the timeline of the children.
    CDisplayObjectContainer::advanceTimeline(player);
}

/////
// exported properties
avm2::Handle_t  CMovieClip::getter_currentFrame(void)
{
    return Handle_t();
}

avm2::Handle_t  CMovieClip::getter_currentFrameLabel(void)
{
    return Handle_t();
}

avm2::Handle_t  CMovieClip::getter_currentLabel(void)
{
    return Handle_t();
}

avm2::Handle_t  CMovieClip::getter_enabled(void)
{
    return Handle_t();
}

avm2::Handle_t  CMovieClip::getter_framesLoaded(void)
{
    return getter_totalFrames();
}

avm2::Handle_t  CMovieClip::getter_totalFrames(void)
{
    if (frames) {
        return avm2::u32ToAtom(static_cast<uint32_t>(frames->frames.size()));
    } else {
        return avm2::u32ToAtom(0);
    }
}

DEFINE_VTABLE(CMovieClip, CSprite)
END_VTABLE()

#if 0
class ClipComponent_t : public CMovieClip::IComponent_t
{
public:
    virtual avm2::ErrorCode_t gotoAndPlayStop(CPlayer &, size_t a_frame, bool play);
    virtual avm2::ErrorCode_t gotoAndPlayStop(CPlayer &, const std::string &, bool play);
    virtual avm2::ErrorCode_t stop(CPlayer &);

    int32_t currentFrame;
    bool    isPlaying;
};

class CMovieClip_impl
{
public:
    std::map<uint32_t, ClipComponent_t> objects;
    CDisplayObjectContainer * containerClass;
};

avm2::ErrorCode_t ClipComponent_t::gotoAndPlayStop(CPlayer & player,
                                                   size_t a_frame, 
                                                   bool play)
{
    return avm2::Err_NotImplemented;
}

avm2::ErrorCode_t ClipComponent_t::gotoAndPlayStop(CPlayer & player,
                                                   const std::string &, 
                                                   bool play)
{
    return avm2::Err_NotImplemented;
}

avm2::ErrorCode_t ClipComponent_t::stop(CPlayer & player)
{
    return avm2::Err_NotImplemented;
}

bool CMovieClip::initObject(avm2::Object_t * obj)
{
    if (impl && obj) {
        map<uint32_t, ClipComponent_t>::iterator it = impl->objects.find(obj->id);
        if (it != impl->objects.end()) {
            return false;
        }
        ClipComponent_t comp;
        comp.isPlaying          = false;
        comp.currentFrame       = -1;
        impl->objects[obj->id]  = comp;
        return baseClass->initObject(obj);
    } else {
        return false;
    }
}

bool CMovieClip::destroyObject(avm2::Object_t * obj)
{
    if (impl && obj) {
        map<uint32_t, ClipComponent_t>::iterator it = impl->objects.find(obj->id);
        if (it != impl->objects.end()) {
            impl->objects.erase(it);
            return baseClass->destroyObject(obj);
        }
    }
    return false;
}

CMovieClip::CMovieClip(CClassObject * baseClass, avm2::CVirtualMachine & vm) : 
    CDisplayClass(baseClass, vm)
{
    impl = new (std::nothrow) CMovieClip_impl();
    impl->containerClass = (CDisplayObjectContainer *)vm.getClass(SWF_DISPLAYOBJECTCONTAINER);
}

CMovieClip::~CMovieClip()
{
    delete impl;
}

CMovieClip::IComponent_t * CMovieClip::getComponent(avm2::Object_t * obj)
{
    if (impl && obj) {
        map<uint32_t, ClipComponent_t>::iterator it = impl->objects.find(obj->id);
        if (it != impl->objects.end()) {
            return &it->second;
        }
    }
    return 0;
}

bool CMovieClip::advanceTimeline(CPlayer & player, avm2::Object_t * obj)
{
    if (ClipComponent_t * comp = (ClipComponent_t * )getComponent(obj)) {
        if (comp->currentFrame == -1) {
            comp->currentFrame = 0;
        } else {
            ++comp->currentFrame;
        }
        // get the frames
        std::shared_ptr<swf::MovieClip> frames = 
            ((CSprite *) baseClass)->getFrames(obj);
        if (frames) {
            if (comp->currentFrame < static_cast<int32_t>(frames->frames.size())) {
                player.ExecuteFrame(frames->frames[comp->currentFrame], obj);
            } else {
                // reset movie clip.
                if (frames->frames.size() > 1) {
                    impl->containerClass->_removeChildren(obj);
                    comp->currentFrame = 0;
                    player.ExecuteFrame(frames->frames[comp->currentFrame], obj);
                }
            }
        }
    }
    return ((CDisplayClass *)baseClass)->advanceTimeline(player, obj);
}

void CMovieClip::draw(IRenderer * renderer, avm2::Object_t * obj)
{
    ((CDisplayClass *)baseClass)->draw(renderer, obj);
}

#endif

}