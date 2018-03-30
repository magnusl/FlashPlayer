#include "CAVM2Player.h"
#include "CTagDecoder.h"
#include <iostream>
#include "swfTags.h"
#include "CDisplayObjectContainer.h"
#include "swfModule.h"
#include <avm2\errorcode.h>
#include "CSWFShape.h"
#include "CDisplayPackage.h"
#include "CEventPackage.h"
#include "CDisplayObject.h"
#include "CSWFShape.h"
#include "CSprite.h"
#include "CMovieClip.h"
#include <avm2\CABCFile.h>

using namespace avm2;
using namespace std;

namespace swf
{

CAVM2Player::CAVM2Player(std::shared_ptr<avm2::io::CBinarySource> source, const swf::FileHeader & hdr) :
    CPlayer(source, hdr),
    displayPackage(0),
    eventPackage(0),
    playState(PLAYBACK_STOPPED),
    playbackTime(0.0f),
    playSpeed(0.0f),
    updateOffset(0.0f)
{
    // create the packages
    displayPackage  = new (std::nothrow) swf::CDisplayPackage(vm);
    eventPackage    = new (std::nothrow) swf::CEventPackage(vm);
    // register the packages with the virtual machine.
    vm.registerPackage(displayPackage);
    vm.registerPackage(eventPackage);

    /////
    // construct the global object.
    if (CClassObject * objClass = vm.findClass(AVM2_OBJECT)) {
        //globalObject = objClass->construct();
    }
}

CAVM2Player::~CAVM2Player()
{
    delete displayPackage;
    delete eventPackage;
}

avm2::CStringTable & CAVM2Player::getStringTable()
{
    return vm.getStringTable();
}

avm2::ErrorCode_t CAVM2Player::Play(size_t frameIndex, float speed)
{
    playSpeed = speed;
    switch(playState) {
    case PLAYBACK_STOPPED:
        if (frameIndex >= swfHdr.FrameCount) {
            return avm2::Err_InvalidParameter;
        }
        if (frameIndex == 0) {
            playState = PLAYBACK_PLAYING;
            // the main timeline is always a subclass of sprite
            if (CSprite * sprite = cast<CSprite>(mainTimeline)) {
                sprite->advanceTimeline(*this);
            }
        } else {
            return avm2::Err_NotImplemented;
        }
        break;
    default:
        return avm2::Err_NotImplemented;
    }
    return avm2::Err_Ok;
}

avm2::ErrorCode_t CAVM2Player::Pause()
{
    return avm2::Err_NotImplemented;
}

/**
 * Returns the frame rate of the currently playing SWF.
 */
float CAVM2Player::GetFrameRate() const
{
    if (swfHdr.FrameRate >= 256) {
        return float(swfHdr.FrameRate) / 256.0f;
    } else {
        return 1.0f;
    }
}

PlayState_t CAVM2Player::GetPlaybackState() const
{
    return playState;
}

avm2::Handle_t CAVM2Player::GetMainTimeline() const
{
    return mainTimeline;
}

void CAVM2Player::GC()
{
    if (CScriptObject * obj = toObject(mainTimeline)) {
        vm.collect(mainTimeline);
    }
}

/**
 * Advances the timeline.
 */
avm2::ErrorCode_t CAVM2Player::Advance(float timeDelta)
{
    timeDelta *= playSpeed;
    if (CSprite * sprite = cast<CSprite>(mainTimeline)) {
        playbackTime += timeDelta;
        updateOffset += timeDelta;
        float secPerFrame = 1.0f / GetFrameRate();

        while(updateOffset >= secPerFrame) 
        {
            sprite->advanceTimeline(*this);
            updateOffset -= secPerFrame;
        }
    }
    return Err_Ok;
}

void CAVM2Player::Draw(IRenderer & renderer)
{
    if (CDisplayObject * obj = cast<CDisplayObject>(mainTimeline))
    {
        std::cout << "We have a main timeline object." << std::endl;
        obj->draw(renderer);
    } else {
        std::cout << "No timeline object." << std::endl;
    }
}

avm2::ErrorCode_t CAVM2Player::Initialize()
{
    size_t currentFrame = 0;

    mainTimelineFrames = make_shared<MovieClip>();
    mainTimelineFrames->frames.resize(swfHdr.FrameCount);

    avm2::ErrorCode_t err;
    do {
        swf::CTagDecoder tag(*swfSource);
        switch(tag.TagCode()) {
        case swf::SwfTag_DoABC:
            {
                doABC(tag);
                break;
            }
        case swf::SwfTag_ShowFrame:
            {
                if (CScriptObject * obj = toObject(mainTimeline)) {
                    ++currentFrame;
                } else {
                    /** We reached the end of the first frame without defining a main timeline class. 
                        Assume that the main timeline is a MovieClip */
                    //mainTimeline = vm.createObject(SWF_MOVIECLIP);
                    assert(false);
                    ++currentFrame;
                }
                break;
            }
        case swf::SwfTag_End:
            {
                // make sure that we have decoded all the frames.
                if (currentFrame != swfHdr.FrameCount) {
                    return avm2::Err_VerfificationError;
                }
                if (swf::CSprite * sprite = cast<CSprite>(mainTimeline)) {
                    sprite->setFrames(mainTimelineFrames);
                }
                // associate the frames with the main timeline object.
                //if (!spriteClass->setFrames(toObject(mainTimeline), mainTimelineFrames)) {
                //  return avm2::Err_VerfificationError;
                //}
                return avm2::Err_Ok;
            }
        case swf::SwfTag_PlaceObject:
        case swf::SwfTag_PlaceObject2:
        case swf::SwfTag_PlaceObject3:
        case swf::SwfTag_RemoveObject:
        case swf::SwfTag_RemoveObject2:
        case swf::SwfTag_SetBackgroundColor:
            ParseAction(tag.TagCode(), tag, mainTimelineFrames->frames[currentFrame]);
            break;
        case swf::SwfTag_DefineShape:
        case swf::SwfTag_DefineButton:
        case swf::SwfTag_DefineFont:
        case swf::SwfTag_DefineText:
        case swf::SwfTag_DefineFontInfo:
        case swf::SwfTag_DefineSound:
        case swf::SwfTag_DefineButtonSound:
        case swf::SwfTag_DefineShape2:
        case swf::SwfTag_DefineShape3:
        case swf::SwfTag_DefineText2:
        case swf::SwfTag_DefineButton2:
        case swf::SwfTag_DefineBitsJPEG3:
        case swf::SwfTag_DefineBitsJPEG2:
        case swf::SwfTag_DefineEditText:
        case swf::SwfTag_DefineSprite:
        case swf::SwfTag_DefineFont2:
        case swf::SwfTag_DefineShape4:
        case swf::SwfTag_DefineSceneAndFrameLabelData:
        case swf::SwfTag_DefineFontName:
        case swf::SwfTag_DefineFont4:
        case swf::SwfTag_DefineFont3:
            // parse the definition and add the character to the dictionary.
            ParseDefinition(tag.TagCode(), tag);
            break;
        }
    } while(!swfSource->Eof());

    return avm2::Err_Ok;
}

static inline float fixed2float(int32_t value)
{
    return float(value) / 65536.0f;
}

static inline float GetEulerAngle(const swf::Matrix_Fixed2x3 & a_Matrix)
{
    float a = fixed2float(a_Matrix.RotateSkew1);
    float b = fixed2float(a_Matrix.ScaleY);
    return atan(a / b);
}

static float twips2float(int32_t a_Twip)
{
    return float(a_Twip) / 20.0f;
}

static void getScaleFactors(const swf::Matrix_Fixed2x3 & a_Transformation, 
                            float & a_ScaleX, 
                            float & a_ScaleY)
{
    float a = fixed2float(a_Transformation.ScaleX), b = fixed2float(a_Transformation.RotateSkew1);
    float c = fixed2float(a_Transformation.RotateSkew0), d = fixed2float(a_Transformation.ScaleY);

    a_ScaleX = sqrt((a*a) + (b*b));
    a_ScaleY = sqrt((c*c) + (d*d));
}

/**
 * Sets the correct transformation for a display component.
 */
static void setTransformation(const swf::Matrix_Fixed2x3 & transform,
                              CDisplayObject * displayObject)
{
    float angle = GetEulerAngle(transform);
    displayObject->setRotation(angle * 57.2957f);
    displayObject->setPosition(twips2float(transform.TranslateX), twips2float(transform.TranslateY));
    float scaleX = 1.0f, scaleY = 1.0f;
    if (transform.HasScale) {
        getScaleFactors(transform, scaleX, scaleY);
    }
    displayObject->setScale(scaleX, scaleY);
}

static void cxform2gfx(const CXFormWithAlpha    & cxform, gfx::ColorTransform & cx)
{
    if (cxform.HasMultTerms) {
        cx.MultTerms[0] = float(cxform.RedMultTerm) / 256;
        cx.MultTerms[1] = float(cxform.GreenMultTerm) / 256;
        cx.MultTerms[2] = float(cxform.BlueMultTerm) / 256;
        cx.MultTerms[3] = float(cxform.AlphaMultTerm) / 256;
    } else {
        cx.MultTerms[0] = 1.0f;
        cx.MultTerms[1] = 1.0f;
        cx.MultTerms[2] = 1.0f;
        cx.MultTerms[3] = 1.0f;
    }

    if (cxform.HasAddTerms) {
        cx.AddTerms[0] = float(cxform.RedAddTerm) / 256;
        cx.AddTerms[1] = float(cxform.GreenAddTerm) / 256;
        cx.AddTerms[2] = float(cxform.BlueAddTerm) / 256;
        cx.AddTerms[3] = float(cxform.AlphaAddTerm) / 256;
    } else {
        cx.AddTerms[0] = 0.0f;
        cx.AddTerms[1] = 0.0f;
        cx.AddTerms[2] = 0.0f;
        cx.AddTerms[3] = 0.0f;
    }
}

static inline void setCharacterAttributes(const PlaceObject2_t & place,
                                          CDisplayObject * displayObject)
{
    if (place.PlaceFlagHasMatrix) {
        setTransformation(place.TransformationMatrix, displayObject);
    }
    if (place.PlaceFlagHasName) {
        displayObject->setName(place.Name);
    }
    if (place.PlaceFlagHasClipDepth) {
        displayObject->setClipDepth(place.ClipDepth);
    }
    if (place.PlaceFlagHasColorTransform) {
        gfx::ColorTransform cx;
        cxform2gfx(place.cxform, cx);
        displayObject->setColorTransform(cx);
    }
}

/**
 * Adds a new character as a child to the sprite.
 */
avm2::ErrorCode_t CAVM2Player::doAddCharacter(const PlaceObject2_t & place,
                                              CDisplayObjectContainer * sprite)

{
    // determine what kind of character we are adding
    CDictionary::CharType_t type = swfDictionary.getCharacterType(place.CharacterId);
    if (type == CDictionary::CHAR_INVALID) {
        return Err_InvalidParameter;
    }

    Handle_t charHandle;
    switch(type)
    {
    case CDictionary::CHAR_SHAPE:
        {
            /////
            // construct a SWF shape
            if (shared_ptr<gfx::Shape_t> gfxShape = swfDictionary.getShape(place.CharacterId)){
                if (CClassObject * obj = vm.findClass(SWF_EMBEDDEDSHAPE)) {
                    //charHandle = obj->construct();
                    if (CSWFShape * shape = cast<CSWFShape>(charHandle)) {
                        shape->setShape(gfxShape);
                    } else {
                        throw runtime_error("failed to construct a SWF_EMBEDDEDSHAPE instance.");
                    }
                } else {
                    throw runtime_error("failed to get embedded SWF shape class.");
                }
            } else {
                throw runtime_error("invalid character id, does not identify a shape.");
            }
        }
    case CDictionary::CHAR_MOVIECLIP:
        {
            /////
            // construct a SWF movieclip instance.
            if (shared_ptr<swf::MovieClip> mc = swfDictionary.getMovieClip(place.CharacterId)) {
                if (CClassObject * obj = vm.findClass(SWF_MOVIECLIP)) {
                    //charHandle = obj->construct();
                    if (CMovieClip * mcObj = cast<CMovieClip>(charHandle)) {
                        mcObj->setFrames(mc);
                    } else {
                        throw runtime_error("failed to construct a SWF_MOVIECLIP instance.");
                    }
                } else {
                    throw runtime_error("failed to get SWF movieclip class.");
                }
            } else {
                throw runtime_error("invalid character id, does not identify a movieclip.");
            }
            break;
        }
    case CDictionary::CHAR_GLYPHTEXT:
        {
            // TODO: implement.
            assert(false);
            break;
        }
    }

    if (CDisplayObject * displayObject = cast<CDisplayObject>(charHandle)) 
    {
        setCharacterAttributes(place, displayObject);
        sprite->addChildAt(charHandle, place.Depth);
    } else {

    }
    return avm2::Err_Ok;
}

avm2::ErrorCode_t CAVM2Player::doModifyCharacter(const PlaceObject2_t & place, 
                                                 CDisplayObjectContainer * sprite)
{
    Handle_t child = sprite->getChildAt(place.Depth);
    if (CDisplayObject * obj = cast<CDisplayObject>(child))
    {
        if (place.PlaceFlagHasMatrix)
        {
            setCharacterAttributes(place, obj);
        }
    } else {
        return Err_InvalidParameter;
    }
    return Err_Ok;
}

/**
 * Executes a PlaceObject2 SWF tag.
 */
avm2::ErrorCode_t CAVM2Player::doPlaceObject2(const swf::Action & a_Action, avm2::Handle_t sprite)
{
    const PlaceObject2_t & place = a_Action.u.Place2;

    CDisplayObjectContainer * container = cast<CDisplayObjectContainer>(sprite);
    if (!container) {
        return Err_InvalidParameter;
    }

    if (!place.PlaceFlagMove && place.PlaceFlagHasCharacter) {
        // a new character is placed on the display list at the specified depth.
        return doAddCharacter(place, container);
    } else if (place.PlaceFlagMove && !place.PlaceFlagHasCharacter) {
        // The character at the specified depth is modified.
        return doModifyCharacter(place, container);
    } else {
        // the character at the specified depth is removed and a new character is
        // added at that depth.
        return doAddCharacter(place, container);
    }
    return avm2::Err_Ok;
}

avm2::ErrorCode_t CAVM2Player::doABC(avm2::io::CBinarySource & source)
{
    uint32_t flags = source.GetU32();
    while(char c = source.GetU8());

    /////
    // Create a ABC file instance and try to parse the ABC file from the source.
    shared_ptr<abc::CABCFile> abc = 
        make_shared<abc::CABCFile>(getStringTable());
    try {
        abc->Parse(source);
    } catch(runtime_error & err) {
        return avm2::Err_VerfificationError;
    }
    const abc::CAbcMethodDefinition * initScript = abc->GetInitScript();
    if (!initScript) {
        return avm2::Err_VerfificationError;
    }
    /////
    // Execute the initialization script using the global object as the "this" object.
    ErrorCode_t err = vm.executeMethod(initScript, globalObject, globalObject, nullptr);
    if (err != Err_Ok) {
        return avm2::Err_VerfificationError;
    }
    /////
    // Save the parsed script file.
    scriptFiles.push_back(abc);

    return Err_Ok;
}

} // namespace swf