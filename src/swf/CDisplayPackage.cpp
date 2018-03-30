#include "CDisplayPackage.h"
#include "swfModule.h"
#include <assert.h>
#include "CDisplayObjectContainer.h"
#include "CSprite.h"
#include "CSWFShape.h"
#include "CMovieClip.h"
#include "CStaticGlyphText.h"
#include <avm2\CRuntimeClass.h>
#include "CDisplayObject.h"

using namespace avm2;

namespace swf
{

/**
 * The names of the classes
 */
static const char * DisplayClassNames[] = 
{
    "DisplayObject",
    "DisplayObjectContainer",
    "InteractiveObject",
    "Sprite",
    "Shape",
    "MovieClip",
    "GlyphText"
};

/**
 * Constructor for the display package.
 */
CDisplayPackage::CDisplayPackage(avm2::CVirtualMachine & VirtualMachine) : 
    CPackage(VirtualMachine)
{
    classObjects.resize(SWF_DisplayMax);
    classNames.resize(SWF_DisplayMax);

    CStringTable & st = VirtualMachine.getStringTable();
    for(size_t i = 0; i < SWF_DisplayMax; ++i) {
        classNames[i] = st.set(DisplayClassNames[i]);
    }
}

CDisplayPackage::~CDisplayPackage()
{
    for(size_t i = 0; i < SWF_DisplayMax; ++i) {
        delete classObjects[i];
    }
}

/**
 * Returns a class object for the specific class.
 */
avm2::CClassObject * CDisplayPackage::getClassObject(uint32_t id)
{
    switch(id) {
    case SWF_DISPLAYOBJECT:
        // flash.display.DisplayObject
        if (!classObjects[SWF_DisplayObject]) 
        {
            classObjects[SWF_DisplayObject] = new (std::nothrow) CDisplayObject_Class(
                vm.findClass(AVM2_ABCCLASSOBJECT), &CDisplayObject_Class::vtable, vm, vm.findClass(SWF_EVENTDISPATCHER));
        }
        return classObjects[SWF_DisplayObject];
    case SWF_EMBEDDEDSHAPE:
        // flash.display.Shape
        if (!classObjects[SWF_EmbeddedShape]) 
        {
            classObjects[SWF_EmbeddedShape] = new (std::nothrow) CSWFShape_Class(
                vm.findClass(AVM2_ABCCLASSOBJECT), &CSWFShape_Class::vtable, vm, vm.findClass(SWF_DISPLAYOBJECT));
        }
        return classObjects[SWF_EmbeddedShape];
    case SWF_INTERACTIVEOBJECT:
        // flash.display.InteractiveObject
        if (!classObjects[SWF_InteractiveObject]) {
            classObjects[SWF_InteractiveObject] = new (std::nothrow) CInteractiveObject_Class(
                vm.findClass(AVM2_ABCCLASSOBJECT), &CInteractiveObject_Class::vtable, vm, vm.findClass(SWF_DISPLAYOBJECT));
        }
    case SWF_DISPLAYOBJECTCONTAINER:
        // flash.display.DisplayObjectContainer
        if (!classObjects[SWF_DisplayObjectContainer]) {
            classObjects[SWF_DisplayObjectContainer] = new (std::nothrow) CDisplayObjectContainer_Class(
                vm.findClass(AVM2_ABCCLASSOBJECT), &CDisplayObjectContainer_Class::vtable, vm, vm.findClass(SWF_INTERACTIVEOBJECT));
        }
        return classObjects[SWF_DisplayObjectContainer];
    case SWF_SPRITE:
        // flash.display.Sprite
        if (!classObjects[SWF_Sprite]) {
            classObjects[SWF_Sprite] = new (std::nothrow) CSprite_Class(
                vm.findClass(AVM2_ABCCLASSOBJECT), &CSprite_Class::vtable, vm, vm.findClass(SWF_DISPLAYOBJECTCONTAINER));
        }
        return classObjects[SWF_Sprite];
    case SWF_MOVIECLIP:
        // flash.display.MovieClip
        if (!classObjects[SWF_MovieClip]) {
            classObjects[SWF_MovieClip] = new (std::nothrow) CMovieClip_Class(
                vm.findClass(AVM2_ABCCLASSOBJECT), &CMovieClip_Class::vtable, vm, vm.findClass(SWF_SPRITE));
        }
    default:
        return nullptr;
    }
}

CClassObject * CDisplayPackage::getClassObject(const QName & name)
{
    for(size_t i = 0; i < SWF_DisplayMax; ++i) {
        if (name.Name == classNames[i]) {
            return getClassObject(SWF_DISPLAYOBJECT + i);
        }
    }
    return nullptr;
}

/**
 * Returns the unique class identifier
 */
uint32_t CDisplayPackage::getPackageId() const
{
    return SWF_DISPLAY_PACKAGE_ID >> 24;
}

/**
 * Returns a string representing the namespace that the package implements.
 */
const char * CDisplayPackage::getPackageNamespace() const
{
    return "flash.display";
}

} // namespace swf