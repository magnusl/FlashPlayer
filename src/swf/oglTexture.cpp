#include <GL\glew.h>
#include "CGLTexture.h"
#include <stdexcept>

namespace swf
{
namespace ogl
{

CGLTexture::~CGLTexture()   
{ 
    glDeleteTextures(1, &m_Id);
}

unsigned CGLTexture::GetID() const  
{
    return m_Id;
}


CGLTexture1D::CGLTexture1D(Format_t a_Format, void * a_Data, size_t a_Width)
{
    glGenTextures(1, &m_Id);
    glBindTexture(GL_TEXTURE_1D, m_Id);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    switch(a_Format)
    {
    case eRGBA_TEXTURE:
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, a_Width, 0, GL_RGBA, GL_UNSIGNED_BYTE, a_Data);
        break;
    default:
        glDeleteTextures(1, &m_Id);
        throw std::runtime_error("Unsupported texture format.");
    }
    glBindTexture(GL_TEXTURE_1D, 0);
}

CGLTexture1D::~CGLTexture1D()
{
}

CGLTexture2D::CGLTexture2D(Format_t a_Format, void * a_Data, size_t a_Width, size_t a_Height)
{
    glGenTextures(1, &m_Id);
    glBindTexture(GL_TEXTURE_2D, m_Id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // copy the actual data
    switch(a_Format)
    {
    case eRGBA_TEXTURE:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, a_Width, a_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, a_Data);
        break;
    case eRGB_TEXTURE:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, a_Width, a_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, a_Data);
        break;
    default:
        glDeleteTextures(1, &m_Id);
        throw std::runtime_error("Unsupported texture format.");
    }
    glBindTexture(GL_TEXTURE_1D, 0);
}

CGLTexture2D::~CGLTexture2D()
{
}

}
}