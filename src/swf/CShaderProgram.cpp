#include "CShaderProgram.h"
#include <GL/glew.h>
#include <iostream>

namespace swf
{
namespace ogl
{

CShaderProgram::CShaderProgram(int a_ProgramId) : m_ProgramId(a_ProgramId)
{
    int total;
    char name[100];
    GLsizei actual;

    glGetProgramiv( m_ProgramId, GL_ACTIVE_UNIFORMS, &total ); 
    for(int i = 0; i < total; ++i) {
        GLint usize;
        GLenum type;
        glGetActiveUniform(m_ProgramId, i, 100, &actual, &usize, &type, name);
        m_Uniforms[std::string(name)] = i;
    }
}

CShaderProgram::~CShaderProgram()
{
}

int CShaderProgram::GetUniformLocation(const std::string & a_Name) const
{
    std::map<std::string, int>::const_iterator it = m_Uniforms.find(a_Name);
    if (it == m_Uniforms.end()) {
        return -1;
    }
    return it->second;
}

std::shared_ptr<CShaderProgram> CShaderProgram::FromSource(
    const char * a_VertexSource, const char * a_FragmentSource)
{
    GLint hVertexShader = -1, hFragmentShader = -1, hProgram = -1;

    if (!a_VertexSource || !a_FragmentSource) {
        return std::shared_ptr<CShaderProgram>();
    }

    // lambda function for cleanup
    auto cleanup = [&hVertexShader, &hFragmentShader, &hProgram]() {
        if (hVertexShader != -1)    glDeleteShader(hVertexShader);
        if (hFragmentShader != -1)  glDeleteShader(hFragmentShader);
        if (hProgram != -1) {
            glDeleteProgram(hProgram);
            hProgram = -1;
        }
    };

    GLint status;
    
    hVertexShader   = glCreateShader( GL_VERTEX_SHADER );
    hFragmentShader = glCreateShader( GL_FRAGMENT_SHADER );

    glShaderSource(hVertexShader, 1, &a_VertexSource, 0);
    glShaderSource(hFragmentShader, 1, &a_FragmentSource, 0);

    if (hVertexShader == -1 || hFragmentShader == -1) {
        cleanup();
        return std::shared_ptr<CShaderProgram>();
    }
        
    glCompileShader(hVertexShader);
    glGetShaderiv(hVertexShader, GL_COMPILE_STATUS, &status);
    
    if (status == GL_FALSE)     // failed to compile vertex shader
    {
        char infoLog[1024];
        glGetShaderInfoLog(hVertexShader, sizeof(infoLog), NULL, infoLog);
        cleanup();
        return std::shared_ptr<CShaderProgram>();
    }

    glCompileShader(hFragmentShader);
    glGetShaderiv(hFragmentShader, GL_COMPILE_STATUS, &status);
        
    if (status == GL_FALSE) {   // failed to compile fragment shader
        char infoLog[1024];
        glGetShaderInfoLog(hFragmentShader, sizeof(infoLog), NULL, infoLog);
        cleanup();
        return std::shared_ptr<CShaderProgram>();
    }

    hProgram = glCreateProgram();
    glAttachShader( hProgram, hVertexShader );
    glAttachShader( hProgram, hFragmentShader );
    glBindFragDataLocation( hProgram, 0, "vFragColor");

    /**
     * Link the program
     */
    glLinkProgram( hProgram );
    glGetProgramiv( hProgram, GL_LINK_STATUS, &status);

    /**
     * Don't need the shaders anymotr
     */
    glDeleteShader( hVertexShader );
    glDeleteShader( hFragmentShader );
    hVertexShader = -1;
    hFragmentShader = -1;

    if (status == GL_FALSE)     // linking failed.
    {
        cleanup();
        return std::shared_ptr<CShaderProgram>();
    }

    return std::make_shared<CShaderProgram>(hProgram);
}

}
}