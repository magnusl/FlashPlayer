#ifndef _IRENDERER_H_
#define _IRENDERER_H_

#include "gfxShape.h"
#include <Eigen/Dense>
#include "gfxColorTransform.h"

namespace swf
{

/**
 * Interface class for SWF renderers
 */
class IRenderer
{
public:
    // initializes the renderer
    virtual bool initialize(size_t width, size_t height) = 0;
    // resizes
    virtual bool resize(size_t width, size_t height) = 0;
    // draws the shape using the supplied transformation and alpha.
    virtual void draw(std::shared_ptr<gfx::Shape_t> shape, const Eigen::Matrix3f & transform, float alpha) = 0;
    // flushes the command pipeline
    virtual void flush(void) = 0;
    // pushes a shape that will act as a clip mask.
    virtual void pushClipMask(std::shared_ptr<gfx::Shape_t> shape,
        const Eigen::Matrix3f & transform) = 0;
    // pops the last pushed clip mask.
    virtual void popClipMask() = 0;
    virtual void pushColorTransform(const gfx::ColorTransform &) = 0;
    virtual void popColorTransform() = 0;
};

}

#endif