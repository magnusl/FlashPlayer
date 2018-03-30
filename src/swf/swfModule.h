#ifndef _SWFMODULE_H_
#define _SWFMODULE_H_

namespace swf
{

#define SWF_PACKAGE_BASE    (1)

#define SWF_EVENTS_PACKAGE_ID   ((SWF_PACKAGE_BASE) << 24)
#define SWF_DISPLAY_PACKAGE_ID  ((SWF_PACKAGE_BASE + 1) << 24)

/*****************************************************************************/
/*                                  flash.display                            */
/*****************************************************************************/
enum {
    SWF_DisplayObject,
    SWF_DisplayObjectContainer,
    SWF_InteractiveObject,
    SWF_Sprite,
    SWF_EmbeddedShape,
    SWF_MovieClip,
    SWF_GlyphText,

    /////
    // This must be the last one.
    SWF_DisplayMax
};

enum {
    SWF_DISPLAYOBJECT           = (SWF_DISPLAY_PACKAGE_ID | SWF_DisplayObject),
    SWF_DISPLAYOBJECTCONTAINER  = (SWF_DISPLAY_PACKAGE_ID | SWF_DisplayObjectContainer),
    SWF_INTERACTIVEOBJECT       = (SWF_DISPLAY_PACKAGE_ID | SWF_InteractiveObject),
    SWF_SPRITE                  = (SWF_DISPLAY_PACKAGE_ID | SWF_Sprite),
    SWF_EMBEDDEDSHAPE           = (SWF_DISPLAY_PACKAGE_ID | SWF_EmbeddedShape),
    SWF_MOVIECLIP               = (SWF_DISPLAY_PACKAGE_ID | SWF_MovieClip),
    SWF_GLYPHTEXT               = (SWF_DISPLAY_PACKAGE_ID | SWF_GlyphText)
};

/*****************************************************************************/
/*                                  flash.events                             */
/*****************************************************************************/
enum {
    SWF_EventDispatcher,

    // Must be the last one
    SWF_EventMax
};

enum {
    SWF_EVENTDISPATCHER         = (SWF_EVENTS_PACKAGE_ID) | SWF_EventDispatcher
};

}

#endif