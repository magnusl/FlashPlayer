#include "CGLRenderer.h"
#include "CGLRCEmitter.h"
#include <GL\glew.h>
#include <GL\gl.h>

#include <iostream>

using namespace std;

namespace swf
{
namespace ogl
{

/*****************************************************************************/
/*                                  Forward declarations                     */
/*****************************************************************************/
static void getTransformedBoundry(const gfx::Rect_t<float> & a_Original,
        const Eigen::Matrix3f & a_Transformation,
        gfx::Rect_t<float> & a_TransformedBoundry);
static Eigen::Matrix4f ortho(float left, float right, float bottom, float top, float zNear, float zFar);


/*****************************************************************************/
/*                                  Exported functions                       */
/*****************************************************************************/
// Initializes the OpenGL renderer
bool CGLRenderer::initialize(size_t width, size_t height)
{
    glewInit();

    sceneSize   = gfx::Point_t<uint16_t>(width, height);
    displaySize = sceneSize;

    std::cerr << "cp1" << std::endl;
    gfx::Rect_t<float> bounds(0, (float)width, 0, (float)height);
    // create the quad tree
    std::cerr << "cp2" << std::endl;
    meshTree = std::make_shared<gfx::QuadTree<uint32_t, 8> >(bounds);
    std::cerr << "cp3" << std::endl;
    if (!initShaders()) {
        std::cerr << "cp4" << std::endl;
        return false;
    }
    std::cerr << "cp3" << std::endl;
    projectionMatrix = ortho(0.0f, float(width), float(height), 0.0f, 0.0f, 1.0f);
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);
    std::cerr << "cp4" << std::endl;
    return true;
}

bool CGLRenderer::resize(size_t width, size_t height)
{
    displaySize = gfx::Point_t<uint16_t>(width, height);
    gfx::Rect_t<float> bounds(0, (float)width, 0, (float)height);
    meshTree = std::make_shared<gfx::QuadTree<uint32_t, 8> >(bounds);
    projectionMatrix = ortho(0.0f, float(width), float(height), 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, width, height);
    return true;
}

// Performs the actual drawing by performing a topological sort and then
// executing the generated commands.
void CGLRenderer::flush()
{
    if (!drawnMeshes.empty()) {
        std::cout << "We have commands to send." << std::endl;
        list<RenderingCommand> cmds;
        CGLRCEmitter emitter(*this, cmds, projectionMatrix);    // create a command emitter
        TopologicalSort(drawnMeshes, emitter);                  // perform the topological sort and generate commands
        executeCommands(cmds);                                  // execute the actual rendering commands.
        drawnMeshes.clear();
        meshTree->reset();
        syncPoints.clear();
    } else {
        std::cout << "No commands to render." << std::endl;
    }
}

std::shared_ptr<ogl::CShaderProgram> CGLRenderer::GetShader(size_t index)
{
    if (index >= shaders.size()) {
        return std::shared_ptr<ogl::CShaderProgram>();
    }
    return shaders[index];
}

// Draws a shape with a specific transformation and alpha value
void CGLRenderer::draw(std::shared_ptr<gfx::Shape_t> shape,
        const Eigen::Matrix3f & transform, float alpha)
{
    std::shared_ptr<ogl::Shape>  glShape = getCachedShape(shape, transform);
    if (glShape) {
        float fScaleX, fScaleY;
        getScaling(fScaleX, fScaleY);
        Eigen::Matrix3f mat =  Eigen::Scaling(fScaleX, fScaleY, 1.0f) * transform;
        draw(glShape, mat, alpha);
    }
}

// draws a OpenGL shape
void CGLRenderer::draw(std::shared_ptr<ogl::Shape> shape,
                       const Eigen::Matrix3f & transform, 
                       float alpha)
{
    if (shape) {
        for(size_t i = 0, num = shape->meshes.size(); i < num; ++i) {
            draw(shape->meshes[i], transform, alpha);
        }
    }
}

void CGLRenderer::draw(std::shared_ptr<Mesh> mesh,
        const Eigen::Matrix3f & transform,
        float alpha)
{
    ogl::Drawable drawable;

    if (!colorTransforms.empty()) {
        drawable.mesh.hasColorTransformation = true;
        drawable.mesh.colorTransform         = colorTransforms.front();
    } else {
        drawable.mesh.hasColorTransformation    = false;
    }
    drawable.mesh.transformation            = transform;
    drawable.mesh.alpha                     = alpha;
    drawable.mesh.mesh                      = mesh;
    drawable.Type                           = Drawable::eMESH;
    drawable.Bounds                         = mesh->m_ShapeBounds;
    draw(drawable, transform);
}

void CGLRenderer::pushColorTransform(const gfx::ColorTransform & transform)
{
    colorTransforms.push_front(transform);
}

void CGLRenderer::popColorTransform()
{
    if (!colorTransforms.empty()) {
        colorTransforms.pop_front();
    }
}

void CGLRenderer::draw(const ogl::Drawable & drawable,
                       const Eigen::Matrix3f & transformation)
{
    gfx::Rect_t<float> transformedBounds;
    getTransformedBoundry(drawable.Bounds, transformation, transformedBounds);
    shared_ptr<DependencyNode<Drawable> > node = 
        std::make_shared<DependencyNode<Drawable> >(drawable);
    // add it to the quadtree and check for overlaps
    list<uint32_t> overlapping;
    meshTree->insert(transformedBounds, drawnMeshes.size(), overlapping);
    // add dependencies to any overlapping item
    for(list<uint32_t>::iterator it = overlapping.begin();
        it != overlapping.end();
        it++)
    {
        node->AddDependency(drawnMeshes[*it].get());
    }
    sync(node);                     // add dependencies to previous clips
    drawnMeshes.push_back(node);    // add the node.

}

void CGLRenderer::sync(shared_ptr<DependencyNode<Drawable> > node)
{
    for(list<shared_ptr<DependencyNode<Drawable> > > ::iterator it = syncPoints.begin();
        it != syncPoints.end();
        it++)
    {
        node->AddDependency(it->get());
    }
}

static float getScaleFactor(const Eigen::Matrix3f & a_Transformation)
{
    return 1.0f;
}

size_t CGLRenderer::getNextCacheIndex()
{
    size_t num = cachedShapes.size();
    cachedShapes.resize(num + 1);
    return num;
}

void CGLRenderer::getScaling(float & fSx, float & fSy)
{
    if (!(displaySize == sceneSize)) {
        fSx = float(displaySize.X) / float(sceneSize.X);
        fSy = float(displaySize.Y) / float(sceneSize.Y);
    } else {
        fSx = 1.0f;
        fSy = 1.0f;
    }
}

// returns a shape that is suitable for rendering using the supplied transformation.
std::shared_ptr<ogl::Shape> 
    CGLRenderer::getCachedShape(std::shared_ptr<gfx::Shape_t> gfxShape,
    const Eigen::Matrix3f & transformation)
{

    float sx, sy, ts;
    ts = getScaleFactor(transformation);
    getScaling(sx, sy);

    float maxScale = std::max(sx, std::max(sy, ts));

    float error = 10.0f / maxScale;
    
    // get the index for the shape, or create a new entry for it.
    if (gfxShape->UserData == -1) {
        gfxShape->UserData = getNextCacheIndex();
    }
    std::vector<std::shared_ptr<ogl::Shape> > & cached = 
            cachedShapes[gfxShape->UserData];
    // check if any of the shapes are suitable
    for(size_t i = 0; i < cached.size(); ++i) {
        if (cached[i]->errorTolerance <= error) {
            return cached[i];
        }
    }
    // no suitable shape    
    vector<gfx::Mesh> meshes;
    if (!triangulateShape(*gfxShape, meshes, error)) {
        throw std::runtime_error("Failed to triangulate shape.");
    }
    std::shared_ptr<ogl::Shape> shape = CreateOGLShape(meshes);
    if (!shape) {
        return std::shared_ptr<ogl::Shape>();
    }

    shape->errorTolerance = error;
    cachedShapes[gfxShape->UserData].push_back(shape);
    return shape;
}


// executes a list of rendering commands.
void CGLRenderer::executeCommands(const std::list<RenderingCommand> & cmds)
{
    for(list<RenderingCommand>::const_iterator it = cmds.begin();
        it != cmds.end();
        it++)
    {
        assert(glGetError() == 0);
        switch(it->Type) 
        {
        case RenderingCommand::RC_USE_PROGRAM:      /**< change shader program */
            glUseProgram(it->u.ProgramID);
            m_BoundShader = it->u.ProgramID;
            break;
        case RenderingCommand::RC_BIND_VERTICIES:   /**< Bind and enable buffer containing vertex data */
                glBindBuffer(GL_ARRAY_BUFFER, it->u.verticies.BufferID);
                glEnableVertexAttribArray(it->u.verticies.Location);
                glVertexAttribPointer(it->u.verticies.Location, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
                break;
            case RenderingCommand::RC_BIND_INDICIES:    /**< Bind and enable buffer containing index data */
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, it->u.indicies.BufferID);
                break;
            case RenderingCommand::RC_BIND_TEXCOORDS:
                glBindBuffer(GL_ARRAY_BUFFER, it->u.texcoords.BufferID);
                glEnableVertexAttribArray(it->u.texcoords.Location);
                glVertexAttribPointer(it->u.texcoords.Location, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
                break;
            case RenderingCommand::RC_SET_UNIFORM_FLOAT:    /**< Set uniform variables */
                switch(it->u.uniform.Count) 
                {
                case 1:     glUniform1fv(it->u.uniform.Location, 1, it->u.uniform.Values); break;
                case 4:     glUniform4fv(it->u.uniform.Location, 1, it->u.uniform.Values); break;
                case 16:    glUniformMatrix4fv(it->u.uniform.Location, 1, GL_FALSE, it->u.uniform.Values); break;
                default:    break;
                }
                break;
            case RenderingCommand::RC_BIND_TEXTURE: /**< bind a texture to texture unit 0 */
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_1D, it->u.texture.TextureID);
                glBindSampler(it->u.texture.Sampler, 0);
                break;
            case RenderingCommand::RC_DRAW_ELEMENTS:    /**< emit indexed verticies */
                glDrawElements(it->u.elements.Mode,it->u.elements.Count, it->u.elements.Type, it->u.elements.Indicies);
                break;
            case RenderingCommand::RC_DRAW_TEXT:
                {
#if 0
                    const char * pStr = it->u.text.StringIndex < m_StringCache.size() ? m_StringCache[it->u.text.StringIndex].c_str() : nullptr;
                    if (pStr != nullptr) {
                        float dx;
                        sth_begin_draw(m_FontStash);
                        sth_draw_text(m_FontStash, it->u.text.FontId, it->u.text.Size, 0.0f, 0.0f, pStr, &dx);
                        sth_end_draw_vbo(m_FontStash, 0, m_TextBuffer->GetID());
                    }
#endif
                    break;
                }
            case RenderingCommand::RC_ENABLE_STENCIL_TEST:                  /**< enable stencil testing */
                {
                    glEnable(GL_STENCIL_TEST);                              /**< enable stencil testing */
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);    /**< don't update color buffer */
                    glClear(GL_STENCIL_BUFFER_BIT);                         /**< clear stencil buffer */
                    glStencilFunc(GL_NEVER, 1, 0xFF);
                    glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
                    glStencilMask(0xFF);
                    break;
                }
            case RenderingCommand::RC_STENCIL_TEST:
                {
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                    glStencilMask(0x00);
                    glStencilFunc(GL_EQUAL, 1, 0xFF);
                    break;
                }
            case RenderingCommand::RC_DISABLE_STENCIL_TEST: /**< disable stencil testing */
                {
                    glDisable(GL_STENCIL_TEST);
                    break;
                }
            default:
                break;
        }
    }
}

static void getTransformedBoundry(const gfx::Rect_t<float> & a_Original,
        const Eigen::Matrix3f & a_Transformation,
        gfx::Rect_t<float> & a_TransformedBoundry)
{
    Eigen::Vector3f topLeft2(a_Original.left, a_Original.top, 1);
    Eigen::Vector3f topRight2(a_Original.right, a_Original.top, 1);
    Eigen::Vector3f bottomLeft2(a_Original.left, a_Original.bottom, 1);
    Eigen::Vector3f bottomRight2(a_Original.right, a_Original.bottom, 1);

    Eigen::Vector3f topLeft     = a_Transformation * topLeft2;
    Eigen::Vector3f topRight    = a_Transformation * topRight2;
    Eigen::Vector3f bottomLeft  = a_Transformation * bottomLeft2;
    Eigen::Vector3f bottomRight = a_Transformation * bottomRight2;

    /** construct the boundry based on the new extreme values */
    a_TransformedBoundry.left   = min(topLeft.x(), min(topRight.x(), min(bottomRight.x(), bottomLeft.x())));
    a_TransformedBoundry.right  = max(topLeft.x(), max(topRight.x(), max(bottomRight.x(), bottomLeft.x())));
    a_TransformedBoundry.top    = min(topLeft.y(), min(topRight.y(), min(bottomRight.y(), bottomLeft.y())));
    a_TransformedBoundry.bottom = max(topLeft.y(), max(topRight.y(), max(bottomRight.y(), bottomLeft.y())));
}

static Eigen::Matrix4f ortho(float left, float right, float bottom, float top, float zNear, float zFar)
{
    Eigen::Matrix4f matrix;
    float tx = -(right + left) / (right - left);
    float ty = -(top + bottom) / (top - bottom);
    float tz = -(zFar + zNear) / (zFar - zNear);

    matrix.col(0) = Eigen::Vector4f(2.0f / (right - left), 0.0f, 0.0f, 0);
    matrix.col(1) = Eigen::Vector4f(0.0f, 2.0f / (top - bottom), 0.0f, 0);
    matrix.col(2) = Eigen::Vector4f(0.0f, 0.0f, -2.0f / (zFar - zNear), 0);
    matrix.col(3) = Eigen::Vector4f(tx, ty, tz, 1.0f);

    return matrix;
}

}
}