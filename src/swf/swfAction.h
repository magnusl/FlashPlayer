#ifndef _SWFACTION_H_
#define _SWFACTION_H_

#include "swftypes.h"
#include <string>

namespace swf
{
    typedef enum {
        eUnknownAction,
        eSetBackground,
        ePlaceObject,
        ePlaceObject2,
        ePlaceObject3,
        eRemoveObject,
        eRemoveObject2
    } ActionType_t;

    struct Action
    {
        ActionType_t type;
        union {
            PlaceObject_t       Place;
            PlaceObject2_t      Place2;
            PlaceObject3_t      Place3;
            RemoveObject_t      Remove;
            RemoveObject2_t     Remove2;
        } u;
    };

    struct Frame
    {
        std::string           Label;
        std::vector<uint16_t> ActionIndicies;
    };

    
    struct MovieClip
    {
        std::vector<Frame> frames;
    };
}

#endif