#include "CDisplayObjectContainer.h"

using namespace std;
using namespace avm2;

namespace swf
{

/*****************************************************************************/
/*                                      Class                                */
/*****************************************************************************/
CDisplayObjectContainer_Class::CDisplayObjectContainer_Class(avm2::CClassObject * cObject, 
        avm2::vtable_t * vt,
        avm2::CVirtualMachine & vm, 
        CClassObject * baseType) :
CClassObject(cObject, vt, vm, baseType)
{
    name = QName(vm.getStringTable().set("DisplayObjectContainer"));
}

CDisplayObjectContainer_Class::~CDisplayObjectContainer_Class()
{
}

Handle_t CDisplayObjectContainer_Class::construct(
    const std::vector<Handle_t> & arguments)
{
    return new (std::nothrow) CDisplayObjectContainer(this,
        &CDisplayObjectContainer::vtable);
}

uint32_t CDisplayObjectContainer_Class::getId() const
{
    return SWF_DISPLAYOBJECTCONTAINER;
}

/*****************************************************************************/
/*                                      Instance                             */
/*****************************************************************************/
CDisplayObjectContainer::CDisplayObjectContainer(avm2::CClassObject * cObject, vtable_t * vt) : 
    CInteractiveObject(cObject, vt)
{

}

CDisplayObjectContainer::~CDisplayObjectContainer()
{
    // make sure that our children don't keep a reference to a deleted parent.
    for(auto it = children.begin(); it != children.end(); it++) {
        it->second->setParent(0);
    }
}

// adds a child on top of the stacking order
void CDisplayObjectContainer::addChild(avm2::Handle_t child)
{
    CDisplayObject * dObj = cast<CDisplayObject>(child);
    if (!dObj) {
        return;
    }
    if (children.empty()) {
        addChildAt(child, 0);
    } else {
        addChildAt(child, children.back().first + 1);
    }
}

void CDisplayObjectContainer::addChildAt(avm2::Handle_t child, uint16_t depth)
{
    CDisplayObject * dObj = cast<CDisplayObject>(child);
    if (!dObj) {
        return;
    }
    dObj->setParent(this);  /**< we are now the parent of this child */

    if (children.empty()) {
        children.push_back(pair<uint16_t, CDisplayObject *>(depth, dObj));
    } else {
        bool wasInserted = false;
        for(auto it = children.begin(); it != children.end(); it++) {
            if (it->first == depth) {
                assert(false);
            } else if (depth > it->first) {
                children.insert(it, pair<uint16_t, CDisplayObject *>(depth, dObj));
                wasInserted = true;
                break;
            }
        }
        if (!wasInserted) {
            children.push_back(pair<uint16_t, CDisplayObject *>(depth, dObj));
        }
    }
}

// removes a child at a specific depth
void CDisplayObjectContainer::removeChildAt(uint16_t depth)
{
    for(auto it = children.begin(); it != children.end(); it++) {
        if (it->first == depth) {
            it->second->setParent(0);   /**< not the parent anymore */
            children.erase(it);
            break;
        }
    }
}

avm2::Handle_t CDisplayObjectContainer::getChildAt(uint16_t depth)
{
    for(auto it = children.begin(); it != children.end(); it++) {
        if (it->first == depth) {
            return it->second;
        }
    }
    return avm2::Handle_t();
}

void CDisplayObjectContainer::removeChildren()
{
    for(auto it = children.begin(); it != children.end(); it++) {
        it->second->setParent(0);
    }
    children.clear();
}

///////
// CDisplayObject
bool CDisplayObjectContainer::hitTestPoint(const gfx::Point_t<float> &, bool pixelPerfect)
{
    return false;
}

void CDisplayObjectContainer::draw(IRenderer & renderer)
{
    for(list<pair<uint16_t, CDisplayObject *> >::reverse_iterator rit = children.rbegin();
        rit != children.rend();
        rit++)
    {
        if (rit->second) {
            rit->second->draw(renderer);
        }
    }
}

void CDisplayObjectContainer::advanceTimeline(swf::CPlayer & player)
{
    for(auto it = children.begin(); it != children.end(); it++) {
        if (it->second) {
            it->second->advanceTimeline(player);
        }
    }
}

/////
// GC
void CDisplayObjectContainer::markReachable()
{
    if (reachable) { /**< already visited */
        return;
    }
    reachable = 1;
    for(auto it = children.begin(); it != children.end(); it++) {
        if (it->second) {
            it->second->markReachable();
        }
    }
    // mark all the children as reachable
    CInteractiveObject::markReachable();    /**< call superclass implementation */
}

DEFINE_VTABLE(CDisplayObjectContainer, CInteractiveObject)
END_VTABLE()


#if 0

    // adds a child at a specific depth
    void addChildAt(avm2::Handle_t, uint16_t);
    // removes a child at a specific depth
    void removeChildAt(uint16_t);
    // removes all the children
    void removeChildren();




/**
 * Represents a single instance of a DisplayObjectContainer.
 */
struct ContainerComponent_t {
    list<pair<uint16_t, avm2::Object_t *> > children;
};

/**
 * Internal implementation of the DisplayObjectContainer class.
 */
class CDisplayObjectContainer_Impl
{
public:
    ContainerComponent_t * getComponent(uint32_t);
    std::map<uint32_t, ContainerComponent_t> objects;
};

/**
 * Returns the component associated with a specific instance.
 */
ContainerComponent_t * CDisplayObjectContainer_Impl::getComponent(uint32_t id) {
    std::map<uint32_t, ContainerComponent_t>::iterator it = objects.find(id);
    if (it != objects.end()) {
        return &it->second;
    }
    return 0;
}

/**
 * Constructor, initializes the display container object.
 */
CDisplayObjectContainer::CDisplayObjectContainer(CClassObject * baseClass, avm2::CVirtualMachine & vm) :
    CDisplayClass(baseClass, vm)
{
    displayClass = (swf::CDisplayObject *) vm.getClass(SWF_DISPLAYOBJECT);
    impl = new (std::nothrow) CDisplayObjectContainer_Impl;
}

CDisplayObjectContainer::~CDisplayObjectContainer()
{
    delete impl;
}

/**
 * Initializes a instance of a DisplayObjectContainer.
 */
bool CDisplayObjectContainer::initObject(avm2::Object_t * obj)
{
    if (!obj) {
        return false;
    }
    if (impl->objects.find(obj->id) != impl->objects.end()) {
        return false;
    }
    // just reference it so a entry is created
    impl->objects[obj->id].children.clear();

    return baseClass ? baseClass->initObject(obj) : true;
}

/**
 * Destroys a instance of a DisplayObjectContainer.
 */
bool CDisplayObjectContainer::destroyObject(avm2::Object_t * obj)
{
    if (!obj) {
        return false;
    }
    std::map<uint32_t, ContainerComponent_t>::iterator it = impl->objects.find(obj->id);
    if (it == impl->objects.end()) {
        return false;
    }
    impl->objects.erase(it);

    return baseClass ? baseClass->destroyObject(obj) : true;

}

/*****************************************************************************/
/*                                  Container                                */
/*****************************************************************************/

avm2::Handle_t CDisplayObjectContainer::_getChildAt(avm2::Handle_t sprite, uint16_t depth)
{
    avm2::Object_t * container = toObject(sprite);
    if (container) {
        ContainerComponent_t * data = impl->getComponent(container->id);
        if (data) {
            for(list<pair<uint16_t, avm2::Object_t *> >::iterator it = data->children.begin();
                it != data->children.end();
                it++)
            {
                if (it->first == depth) {
                    return it->second;
                }
            }
        }
    }
    return Handle_t();
}

// Adds a child to the back of the container (top most).
void CDisplayObjectContainer::_addChild(avm2::Handle_t sprite, avm2::Handle_t child)
{
    avm2::Object_t * container = toObject(sprite);
    Object_t * obj             = toObject(child);

    if (!container || !obj) {
        throw std::runtime_error("Invalid parameters, not objects.");
    }
    CDisplayObject::IDisplayComponent * comp = displayClass->GetComponent(obj);
    if (!comp) {
        throw std::runtime_error("Child is not a display object.");
    }
    ContainerComponent_t * data = impl->getComponent(container->id);
    if (data && obj) {
        data->children.push_back(std::pair<uint16_t, avm2::Object_t *>(0, obj));
        comp->setParent(container);
        obj->incWeakRef();
    }
}

// Adds a child at a specific depth
void CDisplayObjectContainer::_addChildAt(avm2::Handle_t sprite, avm2::Handle_t child, uint16_t depth)
{
    avm2::Object_t * container = toObject(sprite);
    Object_t * obj             = toObject(child);

    if (!container || !obj) {
        throw std::runtime_error("Invalid parameters, not objects.");
    }
    CDisplayObject::IDisplayComponent * comp = displayClass->GetComponent(obj);
    if (!comp) {
        throw std::runtime_error("Child is not a display object.");
    }
    ContainerComponent_t * data = impl->getComponent(container->id);
    if (data && obj) {
        bool wasInserted = false;
        for(list<pair<uint16_t, avm2::Object_t *> >::iterator it = data->children.begin();
            it != data->children.end();
            it++)
        {
            if (depth > it->first) {
                data->children.insert(it, std::pair<uint16_t, avm2::Object_t *>(depth, obj));
                wasInserted = true;
                break;
            }
        }
        if (!wasInserted) {
            data->children.push_back(std::pair<uint16_t, avm2::Object_t *>(depth, obj));
        }
        obj->incWeakRef();
    }
    if (obj->softref > 1) {
        assert(false);
    }
    comp->setParent(container);
}

// Removes a child at a specific depth
bool CDisplayObjectContainer::_removeChildAt(avm2::Handle_t sprite, uint16_t depth)
{
    avm2::Object_t * container = toObject(sprite);
    if (!container) {
        throw std::runtime_error("Invalid parameters, not objects.");
    }
    ContainerComponent_t * data = impl->getComponent(container->id);
    if (data) {
        for(list<pair<uint16_t, avm2::Object_t *> >::iterator it = data->children.begin();
            it != data->children.end();
            it++)
        {
            if (it->first == depth) {
                CDisplayObject::IDisplayComponent * comp = displayClass->GetComponent(it->second);
                if (comp) {
                    comp->setParent((Object_t *)0);
                }
                it->second->decWeakRef();
                data->children.erase(it);
                return true;
            }
        }
    }
    return false;
}

bool CDisplayObjectContainer::_contains(avm2::Handle_t container, avm2::Handle_t child)
{
    return true;
}

// removes a specific child from the display container.
bool CDisplayObjectContainer::_removeChild(avm2::Handle_t container, avm2::Handle_t child)
{
    return false;
}

void CDisplayObjectContainer::_removeChildren(avm2::Handle_t sprite)
{
    avm2::Object_t * container = toObject(sprite);
    ContainerComponent_t * data = impl->getComponent(container->id);
    if (data) {
        for(list<pair<uint16_t, avm2::Object_t *> >::iterator it = data->children.begin();
            it != data->children.end();
            it++)
        {
            Object_t * obj = it->second;
            CDisplayObject::IDisplayComponent * comp = displayClass->GetComponent(it->second);
            if (comp) {
                comp->setParent((Object_t *)0);
            }
            obj->decWeakRef();
            it = data->children.erase(it);
            if (it == data->children.end()) {
                break;
            }
        }
    }
}

bool CDisplayObjectContainer::advanceTimeline(CPlayer & player, avm2::Object_t * obj)
{
    ContainerComponent_t * data = impl->getComponent(obj->id);
    if (data) {
        for(list<pair<uint16_t, avm2::Object_t *> >::iterator it = data->children.begin();
            it != data->children.end();
            it++)
        {
            if (avm2::Object_t * obj = it->second) {
                // call the correct implementation to advance the timeline.
                swf::CDisplayClass * dc = (swf::CDisplayClass *) virtualMachine.getClass(obj->type);
                dc->advanceTimeline(player, obj);
            } else {
                return false;
            }
        }
    }
    return true;
}

// Draws the display object container.
void CDisplayObjectContainer::draw(IRenderer * renderer, avm2::Object_t * object)
{
    uint16_t cliplayer = 0;
    ContainerComponent_t * data = impl->getComponent(object->id);
    if (data) {
        // check if we should apply color transformtion.
        CDisplayObject::IDisplayComponent * dContainer = displayClass->GetComponent(object);
        if (dContainer && dContainer->hasColorTransform()) {
            renderer->pushColorTransform(dContainer->getColorTransform());
        }
        for(list<pair<uint16_t, avm2::Object_t *> >::reverse_iterator it = data->children.rbegin();
            it != data->children.rend();
            it++)
        {
            if (avm2::Object_t * obj = it->second) {
                CDisplayObject::IDisplayComponent * dc = displayClass->GetComponent(obj);
                if (dc) {
                    swf::CDisplayClass * drawClass = (swf::CDisplayClass *) virtualMachine.getClass(obj->type);
                    uint16_t clipDepth = dc->getClipDepth();
                    if (clipDepth) {
                        cliplayer = it->first + clipDepth;
                        // use this object as a clip mask
                        drawClass->useAsClipMask(renderer, obj);
                    } else {
                        if (it->first < cliplayer) {
                            // this object is clipped
                        } else if (cliplayer) {
                            renderer->popClipMask();    // disable clipping
                        }
                        drawClass->draw(renderer, obj);
                    }
                }
            }
        }
        if (dContainer && dContainer->hasColorTransform()) {
            renderer->popColorTransform();
        }
    }
}

#endif
}