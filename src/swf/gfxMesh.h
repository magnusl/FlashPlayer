#ifndef _GFXMESH_H_
#define _GFXMESH_H_

#include "gfxPoint.h"
#include "gfxGradient.h"
#include "gfxTriangle.h"
#include "gfxRect.h"
#include "gfxShape.h"

#include <vector>

namespace swf
{
namespace gfx
{

/**
 * A material that can be applied to a mesh.
 */
struct Material {
    enum {
        MATERIAL_SOLID_COLOR,           /**< Mesh is rendered with a solid color */
        MATERIAL_TEXTURE,               /**< The mesh is textured */
        MATERIAL_LINEAR_GRADIENT,       /**< Linear gradient */
        MATERIAL_RADIAL_GRADIENT
    } MaterialType;

    union {
        float           Color[4];
        Gradient_t      Gradient;
    };

    bool operator ==(const Material & a_Material) const {
        if (MaterialType == a_Material.MaterialType) {
            switch(MaterialType) {
            case MATERIAL_SOLID_COLOR:
                for(size_t i = 0; i < 4; ++i) {
                    if (Color[i] != a_Material.Color[i]) {
                        return false;
                    }
                }
                return true;
            case MATERIAL_TEXTURE:
                return false;
            case MATERIAL_LINEAR_GRADIENT:
            case MATERIAL_RADIAL_GRADIENT:
                return Gradient == a_Material.Gradient;
            default:
                return false;
            }
        } else {
            return false;
        }
    }
};  

struct Mesh
{
    std::vector<gfx::Point_t<float> > VertexData;       /**< Vertex data for mesh */
    std::vector<gfx::Point_t<float> > TextureCoords;    /**< Texture coordinates */
    std::vector<gfx::Triangle_t>      Indicies;         /**< Indicies into vertex and texcoords */
    Material                          Material;         /**< The material of this mesh */
    Rect_t<float>                     Bounds;           /**< The boundry of the mesh */

    // returns the memory footprint of the mesh data, in bytes.
    size_t GetMemoryFootprint() const;
};

/**
 * Triangulates a shape to one or more triangles meshes.
 */
bool triangulateShape(const gfx::Shape_t & shape, std::vector<gfx::Mesh> & meshes, float ErrorTolerance);


}
}

#endif