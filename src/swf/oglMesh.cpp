#include "oglMesh.h"
#include "CGLTexture.h"
#include "gfxGradient.h"
#include "gfxMesh.h"
#include <memory>
#include <GL\glew.h>

using namespace std;

namespace swf
{
namespace ogl
{

static bool generateTexture(const gfx::Gradient_t & a_Gradient, uint8_t a_Texture[], size_t a_TextureSize);


std::shared_ptr<ogl::Shape>
    CreateOGLShape(const std::vector<gfx::Mesh> & meshes)
{
    shared_ptr<ogl::Shape> shape = make_shared<ogl::Shape>();
    if (!meshes.size()) {
        return shape;
    }
    shape->meshes.resize(meshes.size());
    for(size_t i = 0; i < meshes.size(); ++i) {
        if (!(shape->meshes[i] = CreateOGLMesh(meshes[i]))) {
            return std::shared_ptr<ogl::Shape>();
        }
    }
    return shape;
}

/**
 * Creates a OpenGL mesh from a gfx::Mesh.
 */
std::shared_ptr<ogl::Mesh> CreateOGLMesh(const gfx::Mesh & gfxMesh)
{
    shared_ptr<ogl::Mesh> glMesh = make_shared<ogl::Mesh>();
    if (gfxMesh.VertexData.empty() || gfxMesh.Indicies.empty()) {
        return shared_ptr<ogl::Mesh>();
    }
    glMesh->m_Verticies = std::make_shared<ogl::CGLVertexBuffer>(&gfxMesh.VertexData[0],
            gfxMesh.VertexData.size() * sizeof(gfx::Point_t<float>), GL_STATIC_DRAW);

    glMesh->m_Indicies = std::make_shared<CGLIndexBuffer>(&gfxMesh.Indicies[0],
            gfxMesh.Indicies.size() * sizeof(gfx::Triangle_t));

    if (!gfxMesh.TextureCoords.empty()) {
        glMesh->m_TexCoords = std::make_shared<CGLVertexBuffer>(&gfxMesh.TextureCoords[0],
            gfxMesh.TextureCoords.size() * sizeof(gfx::Point_t<float>), GL_STATIC_DRAW);
    }

    glMesh->m_Material      = gfxMesh.Material;
    glMesh->m_NumIndicies   = gfxMesh.Indicies.size() * 3;
    glMesh->m_Filled        = true;
    glMesh->m_ShapeBounds   = gfxMesh.Bounds;

    switch(gfxMesh.Material.MaterialType)
    {
    case gfx::Material::MATERIAL_LINEAR_GRADIENT:
    case gfx::Material::MATERIAL_RADIAL_GRADIENT:
        {
            uint8_t texture[1024];
            generateTexture(glMesh->m_Material.Gradient, texture, 1024);
            glMesh->m_Texture = std::make_shared<CGLTexture1D>(CGLTexture::eRGBA_TEXTURE, texture, 256);
            break;
        }
    }
    return glMesh;
}

static void interpolatePoints(const gfx::GradientPoint_t & a_Start, 
                       const gfx::GradientPoint_t & a_End,
                       uint8_t a_Texture[])
{
    size_t num = a_End.Ratio - a_Start.Ratio;
    if (!num) {
        return;
    }

    float dt[4];
    dt[0] = (a_End.Color[0] - a_Start.Color[0]) / num;
    dt[1] = (a_End.Color[1] - a_Start.Color[1]) / num;
    dt[2] = (a_End.Color[2] - a_Start.Color[2]) / num;
    dt[3] = (a_End.Color[3] - a_Start.Color[3]) / num;
    size_t c = 0;
    size_t index = a_Start.Ratio * 4;
    for(size_t i = a_Start.Ratio; i < a_End.Ratio; ++i)
    {
        a_Texture[index++] = (uint8_t) ((a_Start.Color[0] + dt[0] * c) * 255);
        a_Texture[index++] = (uint8_t) ((a_Start.Color[1] + dt[1] * c) * 255);
        a_Texture[index++] = (uint8_t) ((a_Start.Color[2] + dt[2] * c) * 255);
        a_Texture[index++] = (uint8_t) ((a_Start.Color[3] + dt[3] * c) * 255);
        ++c;
    }
}

static bool generateTexture(const gfx::Gradient_t & a_Gradient, uint8_t a_Texture[], size_t a_TextureSize)
{
    if ((a_TextureSize < 1024) || (a_Gradient.Count == 0)) {
        return false;
    }
    /** a dummy start point that will be used if the first ratio isn't zero */
    gfx::GradientPoint_t start = {{0.0f, 0.0f,0.0f, 1.0f}, 0};
    gfx::GradientPoint_t stop = {{0.0f, 0.0f,0.0f, 0.0f}, 255};

    if  (a_Gradient.Points[0].Ratio != 0) {
        gfx::GradientPoint_t p  = a_Gradient.Points[0];
        p.Ratio                 = 0;
        interpolatePoints(p, a_Gradient.Points[0], a_Texture);
    }
    for(uint8_t i = 0; i < (a_Gradient.Count - 1); ++i)
    {
        interpolatePoints(a_Gradient.Points[i], a_Gradient.Points[i + 1], a_Texture);
    }
    interpolatePoints(a_Gradient.Points[a_Gradient.Count - 1], stop, a_Texture);
    return true;
}

}

}