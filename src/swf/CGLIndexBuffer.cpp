#include "CGLIndexBuffer.h"
#include <GL/glew.h>

namespace swf
{
namespace ogl
{

CGLIndexBuffer::CGLIndexBuffer(const void * a_Indices, size_t a_Size)
{
    glGenBuffers(1, &m_BufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, a_Size, a_Indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

CGLIndexBuffer::~CGLIndexBuffer()
{
    glDeleteBuffers(1, &m_BufferId);
}

}
}