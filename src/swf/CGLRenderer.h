#ifndef _CGLRENDERER_H_
#define _CGLRENDERER_H_

#include "IRenderer.h"
#include "CShaderProgram.h"
#include "oglRenderCommand.h"
#include "depgraph.h"
#include "oglDrawable.h"
#include "gfxQuadTree.h"

#include <list>

namespace swf
{
namespace ogl
{

typedef enum {
    eSOLID_COLOR_SHADER,
    e1D_TEXTURE_SHADER,
    e2D_TEXTURE_SHADER,
    eRADIAL_1D_TEXTURE_SHADER,
    eTEXT_SHADER,
    eSTENCIL_SHADER,
    eSOLID_COLOR_SHADER_CX,         /**< solid color with color transform */
    e1D_TEXTURE_SHADER_CX,          /**< 1d texture shader with color transform */
    e2D_TEXTURE_SHADER_CX,          /**< 2d texture shader with color transform */
    eRADIAL_1D_TEXTURE_SHADER_CX,   /**< radial texture shader with color transform */
    eSHADER_MAX                     /**< Must be the last */
} ShaderIndex;

typedef enum {
    ePROJECTION_UNIFORM,        /**< Projection matrix */
    eTRANSFORMATION_UNIFORN,    /**< Transformation matrix */
    eCOLOR_UNIFORM              /**< Color */
} ShaderUniforms;

/**
 * SWF renderer using OpenGL 2.0
 */
class CGLRenderer : public IRenderer
{
public:
    virtual bool initialize(size_t width, size_t height);
    virtual bool resize(size_t width, size_t height);

    // Draws a shape with a specific transformation and alpha value
    virtual void draw(std::shared_ptr<gfx::Shape_t> shape, const Eigen::Matrix3f & transform, float alpha);
    virtual void flush(void);
    std::shared_ptr<ogl::CShaderProgram> GetShader(size_t);
    void executeCommands(const std::list<RenderingCommand> &);
    virtual void pushClipMask(std::shared_ptr<gfx::Shape_t> shape, const Eigen::Matrix3f & transform);
    virtual void popClipMask();
    virtual void pushColorTransform(const gfx::ColorTransform &);
    virtual void popColorTransform();

protected:
    bool initShaders();
    // returns a shape that is suitable for rendering using the supplied transformation.
    std::shared_ptr<ogl::Shape> getCachedShape(std::shared_ptr<gfx::Shape_t>,
        const Eigen::Matrix3f & a_Transformation);

    // draws a shape, which has one or more meshes
    void draw(std::shared_ptr<ogl::Shape> shape,
        const Eigen::Matrix3f & transform, float alpha);
    // draws a single mesh
    void draw(std::shared_ptr<Mesh> mesh,
        const Eigen::Matrix3f & transform, float alpha);
    // "draws" a drawable item.
    void draw(const ogl::Drawable & drawable,
        const Eigen::Matrix3f & transformation);
    // syncs
    void sync(std::shared_ptr<DependencyNode<Drawable> > a_Node);

    size_t getNextCacheIndex();

private:

    void getScaling(float & fSx, float & fSy);

    gfx::Point_t<uint16_t>                                              sceneSize;
    gfx::Point_t<uint16_t>                                              displaySize;

    std::shared_ptr<DependencyNode<Drawable> >                          currentClipping;
    std::vector<std::shared_ptr<CShaderProgram> >                       shaders;
    unsigned int vertexArrayID;
    unsigned int m_BoundShader;
    std::vector<std::shared_ptr<swf::DependencyNode<ogl::Drawable> > >  drawnMeshes;
    std::shared_ptr<gfx::QuadTree<uint32_t, 8> >                        meshTree;
    Eigen::Matrix4f                                                     projectionMatrix;
    std::vector<std::vector<std::shared_ptr<ogl::Shape> > >             cachedShapes;
    std::list<std::shared_ptr<DependencyNode<Drawable> > >              syncPoints;
    std::list<gfx::ColorTransform>                                      colorTransforms;
};

}
}

#endif