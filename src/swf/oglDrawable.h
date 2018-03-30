#ifndef _OGLDRAWABLE_H_
#define _OGLDRAWABLE_H_

#include "oglMesh.h"
#include <Eigen\Dense>
#include "gfxRect.h"
#include "gfxColorTransform.h"

namespace swf
{
namespace ogl
{

struct Drawable {
    enum {
        eMESH,
        eTEXT,
        eRECT,
        eCLIP_MASK,
        eCLIP_STOP
    } Type;

    struct {
        gfx::ColorTransform         colorTransform;
        std::shared_ptr<ogl::Mesh>  mesh;
        Eigen::Matrix3f             transformation;
        float                       alpha;
        bool                        hasColorTransformation;
    } mesh;

    struct {
        int                 FontHandle;
        size_t              String;
        float               TextSize;
        Eigen::Matrix3f     Transformation;
    } text;

    struct {
        std::vector<std::shared_ptr<ogl::Mesh>> Meshes;
        Eigen::Matrix3f         Transformation;
    } clipping;

    gfx::Rect_t<float>      Bounds;
};

}
}

#endif