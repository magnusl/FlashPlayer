#ifndef _OGLMESH_H_
#define _OGLMESH_H_

#include "CGLIndexBuffer.h"
#include "CGLVertexBuffer.h"
#include "CGLTexture.h"
#include "gfxMesh.h"
#include <vector>
#include <memory>

namespace swf
{
namespace ogl
{

/**
 * \brief   OpenGL mesh with index/vertex buffer and material which can be
 *      rendered by a renderer.
 */
struct Mesh {
    std::shared_ptr<CGLIndexBuffer>     m_Indicies;     /**< indicies into verticies and texcoords */
    std::shared_ptr<CGLVertexBuffer>    m_Verticies;    /**< vertex data */
    std::shared_ptr<CGLVertexBuffer>    m_TexCoords;    /**< texture coordinates */
    gfx::Material                       m_Material;
    size_t                              m_NumIndicies;
    gfx::Rect_t<float>                  m_ShapeBounds;  /**< The bounds for this shape, untransformed */
    std::shared_ptr<CGLTexture>         m_Texture;      /**< Texture used by this shape */
    bool                                m_Filled;
};

/**
 * A shape with one or more meshes.
 */
struct Shape
{
    float errorTolerance;
    std::vector<std::shared_ptr<Mesh> > meshes;
};

/**
 * Creates a OpenGL mesh from a gfx::Mesh.
 */
std::shared_ptr<ogl::Mesh> CreateOGLMesh(const gfx::Mesh & gfxMesh);

/**
 * Creates a OpenGL shape from a set of gfx::Meshes
 */
std::shared_ptr<ogl::Shape> CreateOGLShape(const std::vector<gfx::Mesh>  & meshes);

}
}

#endif