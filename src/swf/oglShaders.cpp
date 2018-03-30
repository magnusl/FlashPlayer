/**
 * \file    oglShader.cpp
 * \brief   Defines the OpenGL Shader Language (GLSL) shader programs used by the SWF renderer.
 * \author  Magnus Leksell
 */
#include "CGLRenderer.h"
#include <iostream>

namespace swf
{
namespace ogl
{

const char * g_SolidVertexSource =
    "#version 130\n"
    "uniform mat4 projection;\n"
    "uniform mat4 transform;\n"
    "\n"
    "in vec2 vVertex;\n"
    "void main()"
    "{\n"
    "   gl_Position =  projection * transform * vec4(vVertex, 0, 1.0);\n"
    "}";

const char * g_SolidFragmentSource =
    "#version 130\n"
    "uniform vec4   color;\n"
    "out vec4       vFragColor;\n"
    ""
    "void main() {\n"
    "   vFragColor = color;\n" 
    "}";

const char * g_1DFragmentShader =
    "#version 130\n"
    "out vec4 vFragColor;\n"
    "uniform sampler1D _texture0;\n"
    "smooth in vec2 vVaryingTexCoords;\n"
    "void main()\n"
    "{\n"
    "vFragColor = texture(_texture0, vVaryingTexCoords.s);\n"
    "}";

const char * g_2DFragmentShader =
    "#version 130\n"
     "out vec4 vFragColor;\n"
     "uniform sampler2D _texture0;\n"
     "smooth in vec2 vVaryingTexCoords;\n"
     "void main()\n"
     "{\n"
     "vFragColor = texture(_texture0, vVaryingTexCoords.st);\n"
     "}";

/**
 * Solid color shader with Color transformation
 */
const char * g_SolidFragmentSource_CX =
    "#version 130\n"
    "uniform vec4   color;\n"
    "uniform vec4   vAddTerms;\n"
    "uniform vec4   vMulTerms;\n"
    "out vec4       vFragColor;\n"
    ""
    "void main() {\n"
    "   vFragColor.r = color.r * vMulTerms.r + vAddTerms.r;\n"
    "   vFragColor.g = color.g * vMulTerms.g + vAddTerms.g;\n"
    "   vFragColor.b = color.b * vMulTerms.b + vAddTerms.b;\n"
    "   vFragColor.a = color.a * vMulTerms.a + vAddTerms.a;\n"
    "}";

const char * g_2DFragmentShader_CX =
    "#version 130\n"
    "out vec4 vFragColor;\n"
    "uniform vec4   vAddTerms;\n"
    "uniform vec4   vMulTerms;\n"
    "uniform sampler2D _texture0;\n"
    "smooth in vec2 vVaryingTexCoords;\n"
    "void main()\n"
    "{\n"
    "   vec4 color = texture(_texture0, vVaryingTexCoords.st);\n"
    "   vFragColor.r = color.r * vMulTerms.r + vAddTerms.r;\n"
    "   vFragColor.g = color.g * vMulTerms.g + vAddTerms.g;\n"
    "   vFragColor.b = color.b * vMulTerms.b + vAddTerms.b;\n"
    "   vFragColor.a = color.a * vMulTerms.a + vAddTerms.a;\n"
    "}";

/**
 * 1D fragment shader with color transformation.
 */
const char * g_1DFragmentShader_CX =
    "#version 130\n"
    "out vec4 vFragColor;\n"
    "uniform vec4   vAddTerms;\n"
    "uniform vec4   vMulTerms;\n"
    "uniform sampler1D _texture0;\n"
    "smooth in vec2 vVaryingTexCoords;\n"
    "void main()\n"
    "{\n"
    "   vec4 color = texture(_texture0, vVaryingTexCoords.s);\n"
    "   vFragColor.r = color.r * vMulTerms.r + vAddTerms.r;\n"
    "   vFragColor.g = color.g * vMulTerms.g + vAddTerms.g;\n"
    "   vFragColor.b = color.b * vMulTerms.b + vAddTerms.b;\n"
    "   vFragColor.a = color.a * vMulTerms.a + vAddTerms.a;\n"
    "}";

/**
* Radial fragment shader with color transformation.
*/
const char * g_RadialFragmentSource_CX =
    "#version 130\n"
    "out vec4 vFragColor;\n"
    "uniform vec4   vAddTerms;\n"
    "uniform vec4   vMulTerms;\n"
    "uniform sampler1D _texture0;\n"
    "smooth in vec2 vVaryingTexCoords;\n"
    "void main()\n"
    "{\n"
    "   float s = 0.5 - vVaryingTexCoords.s;\n"
    "   float t = 0.5 - vVaryingTexCoords.t;\n"
    "   float c = sqrt(s*s + t*t);\n"
    "   vec4 color = texture(_texture0, c);\n"
    "   vFragColor.r = color.r * vMulTerms.r + vAddTerms.r;\n"
    "   vFragColor.g = color.g * vMulTerms.g + vAddTerms.g;\n"
    "   vFragColor.b = color.b * vMulTerms.b + vAddTerms.b;\n"
    "   vFragColor.a = color.a * vMulTerms.a + vAddTerms.a;\n"
    "}";

/*************************************************************************/
/*                      Vertex shader for texturing                      */
/*************************************************************************/
const char * g_TextureVertexSource =
    "#version 130\n"
    "uniform mat4   projection;\n"
    "uniform mat4   transform;\n"
    "in vec2        vVertex;\n"
    "in vec2        vTexCoords;\n"
    "smooth out vec2 vVaryingTexCoords;\n"
    "\n"
    "void main()\n"
    "{\n"
    "   gl_Position       = projection * transform * vec4(vVertex, 0, 1.0);\n"
    "   vVaryingTexCoords = vTexCoords;\n"
    "}";

const char * g_TextFragmentShader =
    "#version 130\n"
    "out vec4 vFragColor;\n"
    "uniform sampler2D _texture0;\n"
    "smooth in vec2 vVaryingTexCoords;\n"
    "void main()\n"
    "{\n"
    "   float alpha = texture(_texture0, vVaryingTexCoords.st).r;\n"
    "   vFragColor.a = alpha;\n"
    "   vFragColor.r = 0.0;"
    "   vFragColor.g = 0.0;"
    "   vFragColor.b = 1.0;"
    "}";

const char * g_RadialFragmentSource =
    "#version 130\n"
    "out vec4 vFragColor;\n"
    "uniform sampler1D _texture0;\n"
    "smooth in vec2 vVaryingTexCoords;\n"
    "void main()\n"
    "{\n"
    "   float s = 0.5 - vVaryingTexCoords.s;\n"
    "   float t = 0.5 - vVaryingTexCoords.t;\n"
    "   float c = sqrt(s*s + t*t);\n"
    "   vFragColor = texture(_texture0, c);\n"
    "}";

const char * g_WhiteFragmentSource =
    "#version 130\n"
    "out vec4 vFragColor;\n"
    "void main() {\n"
    "   vFragColor = vec4(1, 1, 1, 1);\n"
    "}";

static struct {
    const char * VertexSource;
    const char * FragmentSource;
} g_ShaderSources[] = {
    {g_SolidVertexSource, g_SolidFragmentSource},       // eSOLID_COLOR_SHADER
    {g_TextureVertexSource, g_1DFragmentShader},        // e1D_TEXTURE_SHADER
    {g_TextureVertexSource, g_2DFragmentShader},        // e2D_TEXTURE_SHADER
    {g_TextureVertexSource, g_RadialFragmentSource},    // eRADIAL_1D_TEXTURE_SHADER
    {g_TextureVertexSource, g_TextFragmentShader},      // eTEXT_SHADER
    {g_SolidVertexSource, g_WhiteFragmentSource},       // eSTENCIL_SHADER

    {g_SolidVertexSource, g_SolidFragmentSource_CX},        // eSOLID_COLOR_SHADER_CX
    {g_TextureVertexSource, g_1DFragmentShader_CX},     // e1D_TEXTURE_SHADER_CX
    {g_TextureVertexSource, g_2DFragmentShader_CX},     // e2D_TEXTURE_SHADER_CX
    {g_TextureVertexSource, g_RadialFragmentSource_CX}, // eRADIAL_1D_TEXTURE_SHADER_CX
};

static float g_RectangleVerticies[] = {
    -0.5, -0.5,
    -0.5, 0.5,
    0.5, 0.5,
    0.5, -0.5
};

static unsigned int g_RectangleIndicies[] = {
    0, 1, 2,
    2, 3, 0
};

static unsigned int g_RectangeStrokeIndicies[] = {
    0, 1, 1, 2, 2, 3, 3, 0
};

/**
 * Initializes the shaders
 */
bool CGLRenderer::initShaders()
{
    shaders.resize(eSHADER_MAX);
    for(size_t i = 0; i < eSHADER_MAX; ++i) {
        std::cout << "compiling shader " << i << std::endl;
        if (!(shaders[i] = 
            CShaderProgram::FromSource(g_ShaderSources[i].VertexSource, g_ShaderSources[i].FragmentSource))) {
            return false;
        }
    }
    return true;
}

}
}