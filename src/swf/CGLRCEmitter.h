#include "IRenderer.h"
#include <list>
#include "oglRenderCommand.h"
#include "oglDrawable.h"
#include "depgraph.h"
#include "CShaderProgram.h"
#include "CGLRenderer.h"

namespace swf
{
namespace ogl
{

/**
 * Emits OpenGL render commands.
 */
class CGLRCEmitter : public swf::ITopologicalSorter_Result<Drawable>
{
public:
    CGLRCEmitter(CGLRenderer & a_Renderer, std::list<RenderingCommand> & a_Commands,
        const Eigen::Matrix4f & a_Projection);

    void OnResultSet(const std::list<swf::DependencyNode<ogl::Drawable> * > & a_Meshes);
    void generate(const ogl::Drawable & a_Mesh);

protected:
    void generateMesh(const ogl::Drawable & a_Mesh);
    void generateText(const ogl::Drawable & a_Mesh);
    void generateClipMask(const ogl::Drawable & a_Mesh);

    void changeShader(int32_t a_Index);
    void setUniform(int Location, const float a_Value[], size_t a_Count);
    void setTransformation(const Eigen::Matrix3f & a_Matrix);
    void indexedDrawCall(std::shared_ptr<ogl::Mesh> a_Mesh);
    void enableTexture(int a_Sampler, uint32_t a_TextureId);
    void bindTexcoords(uint32_t a_BufferID);

protected:
    int32_t     m_CurrentShader;    /**< the current shader program */
    int32_t     m_CurrentTexture;   /**< the current texture */
    uint32_t    m_CurrentVertexBuffer;
    uint32_t    m_CurrentIndiciesBuffer;
    CGLRenderer & m_Renderer;
    std::shared_ptr<ogl::CShaderProgram> m_ShaderPtr;
    std::list<ogl::RenderingCommand> & m_Emitted;
    const Eigen::Matrix4f & m_Projection;
};

}
}