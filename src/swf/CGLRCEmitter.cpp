#include <GL\glew.h>
#include "CGLRCEmitter.h"
#include "CGLRenderer.h"

namespace swf
{
namespace ogl
{

CGLRCEmitter::CGLRCEmitter(CGLRenderer & a_Renderer, 
    std::list<ogl::RenderingCommand> & a_Commands,
    const Eigen::Matrix4f & a_Projection) 
    : m_Renderer(a_Renderer), 
    m_Emitted(a_Commands), 
    m_Projection(a_Projection),
    m_CurrentShader(-1),
    m_CurrentIndiciesBuffer(-1),
    m_CurrentVertexBuffer(-1)
{
}

void expandMatrix(const Eigen::Matrix3f & a_In, Eigen::Matrix4f & a_Out)
{
    const float * data = a_In.data();
    a_Out.col(0) = Eigen::Vector4f(data[0], data[1], 0, 0);
    a_Out.col(1) = Eigen::Vector4f(data[3], data[4], 0, 0);
    a_Out.col(2) = Eigen::Vector4f(0, 0, 1, 0);
    a_Out.col(3) = Eigen::Vector4f(data[6], data[7], 0, 1);
}

/**
 * Returns the index associated with a key.
 */
inline uint16_t keyToIndex(uint64_t a_Key)
{
    return static_cast<uint16_t>(a_Key & 0xfff);    /**< index is 12 bits */
}

/** stores the mesh index in the lower 12 bit of a 64 bit key, 
    this allows for 4096 meshes in each batch */
inline uint64_t indexToKey(uint16_t a_MeshIndex)
{
    return static_cast<uint64_t>(a_MeshIndex) & 0xfff;
}

/** stores the shader program index in the 4 upper bit of a 64 bit key */
inline uint64_t shaderToKey(uint8_t a_ProgramIndex)
{
    return static_cast<uint64_t>(a_ProgramIndex) << 60;
}

/** stores the texture id in the bits 28..60 */
inline uint64_t textureToKey(int32_t a_TextureID)
{
    return static_cast<uint64_t>(a_TextureID) << 28;
}

/**
* \brief    Generates a 64-bit key for a drawn mesh based on it's material
*       and index. This is then used to sort the drawing order of the meshes.
*
* \param    [in] a_Draw     The mesh that the key should be generated from.
* \param    [in] a_Index    The index to encode into the key. Only the lower 12 bits
*       of the index will be used.
*
* \return   The 64-bit key that represents this mesh.
*/
uint64_t getSortKey(const ogl::Drawable & a_Draw, size_t a_Index)
{
    uint64_t key = indexToKey(a_Index);
    if (a_Draw.Type == ogl::Drawable::eMESH) {
        const gfx::Material & mat = a_Draw.mesh.mesh->m_Material;

        switch(mat.MaterialType)
        {
        case gfx::Material::MATERIAL_SOLID_COLOR:
            if (a_Draw.mesh.hasColorTransformation) {
                key |= shaderToKey(ogl::eSOLID_COLOR_SHADER_CX);    /**< singel color shader w/ color transformation */
            } else {
                key |= shaderToKey(ogl::eSOLID_COLOR_SHADER);   /**< singel color shader */
            }
            break;
        case gfx::Material::MATERIAL_TEXTURE:
            if (a_Draw.mesh.hasColorTransformation) {
                key |= shaderToKey(ogl::e2D_TEXTURE_SHADER_CX);
            } else {
                key |= shaderToKey(ogl::e2D_TEXTURE_SHADER);    /**< 2d texture */
            }
            break;
        case gfx::Material::MATERIAL_LINEAR_GRADIENT:
            if (a_Draw.mesh.hasColorTransformation) {
                key |= shaderToKey(ogl::e1D_TEXTURE_SHADER_CX);
            } else {
                key |= shaderToKey(ogl::e1D_TEXTURE_SHADER);
            }
            break;
        case gfx::Material::MATERIAL_RADIAL_GRADIENT:
            if (a_Draw.mesh.hasColorTransformation) {
                key |= shaderToKey(ogl::eRADIAL_1D_TEXTURE_SHADER_CX);
            } else {
                key |= shaderToKey(ogl::eRADIAL_1D_TEXTURE_SHADER);
            }
            break;
        default:
            return 0;                       
        }
    } else if (a_Draw.Type == ogl::Drawable::eTEXT) {
        key |= shaderToKey(ogl::eTEXT_SHADER);
    }
    return key;
}

/**
* \brief    Generates a list of rendering commands to render the supplied meshes in a efficient way.
*
* \param [in] a_Meshes      Meshes that can be rendered independently of each other.
*/
void CGLRCEmitter::OnResultSet(const std::list<swf::DependencyNode<ogl::Drawable> * > & a_Meshes)
{
    size_t i = 0;
    std::vector<uint64_t> sortvec(a_Meshes.size());
    std::vector<const swf::DependencyNode<ogl::Drawable> *> quick(a_Meshes.size()); /**< for quick lookup */

    /** convert each drawn mesh to a key, which can be used to sort it */
    for(auto it = a_Meshes.begin(); it != a_Meshes.end(); it++) {
        sortvec[i] = getSortKey((*it)->Value, i);
        quick[i]   = *it;
        ++i;
    }
    /** sort the meshes in order to minimize state change */
    std::sort(sortvec.begin(), sortvec.end());
    for(size_t i = 0, num = sortvec.size(); i < num; ++i) {
        const swf::DependencyNode<ogl::Drawable> * mesh = quick[keyToIndex(sortvec[i])]; /**< get the mesh to render */
        generate(mesh->Value);                                                            /**< generate the commands to render it */
    }
}

/**
* \brief    Generates the commands required to draw a specific mesh based on the current state.
*
* \param [in] a_Mesh    The mesh to render command for.
*/
void CGLRCEmitter::generate(const ogl::Drawable & a_Draw)
{
    switch(a_Draw.Type) 
    {
    case Drawable::eMESH:       generateMesh(a_Draw); break;
    case Drawable::eTEXT:       generateText(a_Draw); break;
    case Drawable::eCLIP_MASK:  generateClipMask(a_Draw); break;
    case Drawable::eCLIP_STOP:  
        m_Emitted.push_back(RenderingCommand::RC_DISABLE_STENCIL_TEST);
        break;
    }
}

void CGLRCEmitter::generateText(const ogl::Drawable & a_Drawable)
{
    if (m_CurrentShader != eTEXT_SHADER) { /**< need to change shader, and reset state */
        changeShader(eTEXT_SHADER);
    }

    int vertex      = glGetAttribLocation(m_ShaderPtr->GetProgram(), "vVertex");
    int texcoords   = glGetAttribLocation(m_ShaderPtr->GetProgram(), "vTexCoords");

    Eigen::Matrix3f matrix = a_Drawable.text.Transformation;
    matrix.col(1).y() = -matrix.col(1).y();
    setTransformation(matrix);

    ogl::RenderingCommand cmd;
    cmd.Type = ogl::RenderingCommand::RC_DRAW_TEXT;
    cmd.u.text.FontId       = a_Drawable.text.FontHandle;
    cmd.u.text.StringIndex  = a_Drawable.text.String;
    cmd.u.text.Size         = a_Drawable.text.TextSize;
    cmd.u.text.x            = 0.0f;
    cmd.u.text.y            = 0.0f;

    m_Emitted.push_back(cmd);
}


ShaderIndex GetShader(const ogl::Drawable & a_Draw)
{
    const gfx::Material & mat = a_Draw.mesh.mesh->m_Material;
    switch(mat.MaterialType)
    {
    case gfx::Material::MATERIAL_SOLID_COLOR:
        return a_Draw.mesh.hasColorTransformation ? eSOLID_COLOR_SHADER_CX : eSOLID_COLOR_SHADER;
    case gfx::Material::MATERIAL_TEXTURE:
        return a_Draw.mesh.hasColorTransformation ? e2D_TEXTURE_SHADER_CX : e2D_TEXTURE_SHADER;
    case gfx::Material::MATERIAL_LINEAR_GRADIENT:
        return a_Draw.mesh.hasColorTransformation ? e1D_TEXTURE_SHADER_CX : e1D_TEXTURE_SHADER;
    case gfx::Material::MATERIAL_RADIAL_GRADIENT:
        return a_Draw.mesh.hasColorTransformation ? eRADIAL_1D_TEXTURE_SHADER_CX : eRADIAL_1D_TEXTURE_SHADER;
    default:
        return eSHADER_MAX;
    }
}


void CGLRCEmitter::generateMesh(const ogl::Drawable & a_Draw)
{
    ShaderIndex shader = GetShader(a_Draw);
    if (shader == eSHADER_MAX) {
        return;
    } else if (m_CurrentShader != shader) {
        changeShader(shader);
    }

    const gfx::Material & mat = a_Draw.mesh.mesh->m_Material;
    switch(shader)
    {
    case eSOLID_COLOR_SHADER:       /**< solid color with or without color transformation */
    case eSOLID_COLOR_SHADER_CX:
        if(m_ShaderPtr) {
            int location = m_ShaderPtr->GetUniformLocation("color");
            setUniform(location, mat.Color, 4); /**< set color */
        }
        break;
    case e2D_TEXTURE_SHADER:
    case e2D_TEXTURE_SHADER_CX:
        break;
    case e1D_TEXTURE_SHADER:
    case e1D_TEXTURE_SHADER_CX:
        if (m_ShaderPtr) { /**< Enable texturing and set the correct texture */
            if (a_Draw.mesh.mesh->m_Texture && a_Draw.mesh.mesh->m_TexCoords) {
                bindTexcoords(a_Draw.mesh.mesh->m_TexCoords->GetID());
                enableTexture(0, a_Draw.mesh.mesh->m_Texture->GetID());
            }
        }
        break;
    case eRADIAL_1D_TEXTURE_SHADER:
    case eRADIAL_1D_TEXTURE_SHADER_CX:
        if (m_ShaderPtr) { /**< Enable texturing and set the correct texture */
            if (a_Draw.mesh.mesh->m_Texture && a_Draw.mesh.mesh->m_TexCoords) {
                bindTexcoords(a_Draw.mesh.mesh->m_TexCoords->GetID());
                enableTexture(0, a_Draw.mesh.mesh->m_Texture->GetID());
            }
        }
        break;
    }
    if (a_Draw.mesh.hasColorTransformation) { 
        /** set the correct uniforms for the color transformation */
        int addTerms = m_ShaderPtr->GetUniformLocation("vAddTerms");
        int mulTerms = m_ShaderPtr->GetUniformLocation("vMulTerms");
        setUniform(addTerms, a_Draw.mesh.colorTransform.AddTerms, 4); /**< set addition terms */
        setUniform(mulTerms, a_Draw.mesh.colorTransform.MultTerms, 4); /**< set multiplication terms */
    }

    setTransformation(a_Draw.mesh.transformation);  /**< set the transformation matrix for the mesh */
    indexedDrawCall(a_Draw.mesh.mesh);              /**< perform the actual rendering */
}

/**
* \brief    Generates the required commans for enabling support for stencil testing.
*
* \param    [in] a_ClipMask     Defines the clip mask.
*/
void CGLRCEmitter::generateClipMask(const ogl::Drawable & a_ClipMask)
{
    // change to the correct shader.
    if (m_CurrentShader != eSTENCIL_SHADER) {
        changeShader(eSTENCIL_SHADER);
    }
    // enable stencil testing
    m_Emitted.push_back(RenderingCommand::RC_ENABLE_STENCIL_TEST);
    // set the transformation matrix for the mesh
    setTransformation(a_ClipMask.clipping.Transformation);
    // draw the actual meshes that define the stencil mask.
    for(auto it = a_ClipMask.clipping.Meshes.begin(); 
        it != a_ClipMask.clipping.Meshes.end();
        it++)
    {
        indexedDrawCall(*it);
    }
    // the following drawn objects are tested using the stencil mask.
    m_Emitted.push_back(RenderingCommand::RC_STENCIL_TEST);
}

void CGLRCEmitter::setUniform(int a_Location, const float a_Value[], size_t a_Count)
{
    RenderingCommand cmd;
    cmd.Type                = RenderingCommand::RC_SET_UNIFORM_FLOAT;
    cmd.u.uniform.Count     = a_Count;
    cmd.u.uniform.Location  = a_Location,
        memcpy(cmd.u.uniform.Values, a_Value, a_Count * sizeof(float));
    m_Emitted.push_back(cmd);
}

void CGLRCEmitter::enableTexture(int a_Sampler, uint32_t a_TextureId)
{
    RenderingCommand cmd;
    cmd.Type                = RenderingCommand::RC_BIND_TEXTURE;
    cmd.u.texture.Sampler   = a_Sampler;
    cmd.u.texture.TextureID = a_TextureId;
    m_Emitted.push_back(cmd);
}

void CGLRCEmitter::bindTexcoords(uint32_t a_BufferID)
{
    RenderingCommand cmd;
    cmd.Type = RenderingCommand::RC_BIND_TEXCOORDS;
    cmd.u.texcoords.BufferID = a_BufferID;
    cmd.u.texcoords.Location = glGetAttribLocation(m_ShaderPtr->GetProgram(), "vTexCoords");
    m_Emitted.push_back(cmd);
}

void CGLRCEmitter::setTransformation(const Eigen::Matrix3f & a_Matrix)
{
    Eigen::Matrix4f transformation;
    ogl::expandMatrix(a_Matrix, transformation);
    RenderingCommand cmd;
    cmd.Type                    = RenderingCommand::RC_SET_UNIFORM_FLOAT;
    cmd.u.uniform.Count         = 16;
    cmd.u.uniform.Location      = m_ShaderPtr->GetUniformLocation("transform");
    memcpy(cmd.u.uniform.Values, transformation.data(), 16 * sizeof(float));
    m_Emitted.push_back(cmd);
}

void CGLRCEmitter::indexedDrawCall(std::shared_ptr<ogl::Mesh> a_Mesh)
{
    /** first bind the vertex buffer */
    {
        uint32_t vertexBufferId = a_Mesh->m_Verticies->GetID();
        if (m_CurrentVertexBuffer != vertexBufferId) {
            RenderingCommand bindVerticies;
            bindVerticies.Type = RenderingCommand::RC_BIND_VERTICIES;
            bindVerticies.u.verticies.BufferID = vertexBufferId;
            bindVerticies.u.verticies.Location = glGetAttribLocation(m_ShaderPtr->GetProgram(), "vVertex");
            m_Emitted.push_back(bindVerticies);
            m_CurrentVertexBuffer = vertexBufferId;
        }
    }
    /** bind the index buffer */
    {
        uint32_t indiciesBufferId = a_Mesh->m_Indicies->GetID();
        if (m_CurrentIndiciesBuffer != indiciesBufferId) {
            RenderingCommand bindIndicies;
            bindIndicies.Type = RenderingCommand::RC_BIND_INDICIES;
            bindIndicies.u.indicies.BufferID = a_Mesh->m_Indicies->GetID();
            m_Emitted.push_back(bindIndicies);
            m_CurrentIndiciesBuffer = indiciesBufferId;
        }
    }
    /** now perform the actual draw call */
    {
        RenderingCommand cmd;
        cmd.Type                = RenderingCommand::RC_DRAW_ELEMENTS;
        cmd.u.elements.Mode     = a_Mesh->m_Filled ? GL_TRIANGLES : GL_LINES;
        cmd.u.elements.Type     = GL_UNSIGNED_INT;
        cmd.u.elements.Count    = a_Mesh->m_NumIndicies; /**< indicies = triangles * 3 */
        cmd.u.elements.Indicies = 0;
        m_Emitted.push_back(cmd);
    }
}

/**
* \brief    Generates the commands for changing shader, and resets the current state.
*/
void CGLRCEmitter::changeShader(int32_t a_Index)
{
    /** generate command for changing GLSL shader program */
    {
        RenderingCommand cmd;
        m_ShaderPtr = m_Renderer.GetShader((ogl::ShaderIndex) a_Index);
        if (m_ShaderPtr) {
            cmd.Type        = RenderingCommand::RC_USE_PROGRAM;
            cmd.u.ProgramID = m_ShaderPtr->GetProgram();
        }
        m_CurrentShader         = a_Index;
        m_CurrentTexture        = -1;
        m_CurrentIndiciesBuffer = -1;
        m_CurrentVertexBuffer   = -1;
        m_Emitted.push_back(cmd);
    }
    /** generate a command for setting the projection matrix */
    {
        RenderingCommand cmd;
        cmd.Type = RenderingCommand::RC_SET_UNIFORM_FLOAT;
        cmd.u.uniform.Count = 16;
        cmd.u.uniform.Location = m_ShaderPtr->GetUniformLocation("projection");
        memcpy(cmd.u.uniform.Values, m_Projection.data(), 16 * sizeof(float));
        m_Emitted.push_back(cmd);
    }
}

}
}