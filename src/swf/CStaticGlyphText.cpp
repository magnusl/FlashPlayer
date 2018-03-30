#include "CStaticGlyphText.h"
#include "CDictionary.h"
#include "CDisplayObject.h"
#include "swfModule.h"

#include <map>

using namespace std;

namespace swf
{

#if 0
struct TextComponent_t {
    std::shared_ptr<GlyphText_t> textPtr;
    swf::CDictionary * dict;
};

class CStaticGlyphText_Impl
{
public:
    CDisplayObject * displayClass;
    std::map<uint32_t, TextComponent_t> objects;
};

CStaticGlyphText::CStaticGlyphText(CClassObject * baseClass, avm2::CVirtualMachine & vm) :
    CDisplayClass(baseClass, vm)
{
    impl = new (std::nothrow) CStaticGlyphText_Impl;
    impl->displayClass = (CDisplayObject *)vm.getClass(SWF_DISPLAYOBJECT);
}

CStaticGlyphText::~CStaticGlyphText()
{
    delete impl;
}

bool CStaticGlyphText::initObject(avm2::Object_t * obj)
{
    std::map<uint32_t, TextComponent_t>::iterator it = impl->objects.find(obj->id);
    if (it != impl->objects.end()) {
        return false;
    }
    TextComponent_t text;
    text.dict = 0;
    impl->objects[obj->id] = text;

    return baseClass->initObject(obj);
}

bool CStaticGlyphText::destroyObject(avm2::Object_t * obj)
{
    std::map<uint32_t, TextComponent_t>::iterator it = impl->objects.find(obj->id);
    if (it == impl->objects.end()) {
        return false;
    }
    impl->objects.erase(it);
    return baseClass->destroyObject(obj);
}

void CStaticGlyphText::setText(avm2::Handle_t handle,
                               std::shared_ptr<GlyphText_t> text,
                               swf::CDictionary * dictionary)
{
    if (avm2::Object_t * obj = avm2::toObject(handle)) {
        std::map<uint32_t, TextComponent_t>::iterator it = impl->objects.find(obj->id);
        if (it != impl->objects.end()) {
            it->second.textPtr  = text;
            it->second.dict     = dictionary;
        }
    }
}

void CStaticGlyphText::draw(IRenderer * renderer, avm2::Object_t * obj)
{
    if (obj) {
        swf::CDisplayObject::IDisplayComponent * dc = impl->displayClass->GetComponent(obj);
        std::map<uint32_t, TextComponent_t>::iterator objIt = impl->objects.find(obj->id);
        if (objIt != impl->objects.end()) {
            std::shared_ptr<GlyphText_t> text = objIt->second.textPtr;
            if (text) {
                for(list<GlyphTextSegment_t>::iterator it = text->Segments.begin();
                    it != text->Segments.end();
                    it++)
                {
                    swf::CDictionary * dict = objIt->second.dict;
                    if (shared_ptr<gfx::GlyphFont> font = dict->getGlyphFont(it->FontId))
                    {
                        Eigen::Matrix3f transform;
                        dc->getGlobalTransform(transform);
                        if (it->HasYOffset) {
                            transform.col(2).y() += float(it->YOffset) / 20;
                        }
                        if (it->HasXOffset) {
                            transform.col(2).x() += float(it->XOffset) / 20;
                        }
                        float scale = float(it->TextHeight) / 1024;
                        transform.col(0).x() = scale;
                        transform.col(1).y() = scale;
                        transform.col(2).z() = scale;
                        for(size_t i = 0; i < it->Text.size(); ++i)
                        {
                            int16_t advancement;
                            uint32_t index = it->Text[i].GlyphIndex;
                            std::shared_ptr<gfx::Shape_t> glyph = font->getGlyph(index, advancement);
                            if (glyph) {
                                renderer->draw(glyph, transform, dc->getAlpha());
                            }
                            transform.col(2).x() += float(it->Text[i].GlyphAdvance) / 20;
                        }
                    }
                }
            }
        }
    }
}

#endif
}