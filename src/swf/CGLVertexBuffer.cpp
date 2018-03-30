#include "CGLVertexBuffer.h"
#include <GL\glew.h>

namespace swf
{
namespace ogl
{

/**
 * \brief   Constructor for oglVertexBuffer
 * \param [in] a_VertexData The vertex data.
 * \param [in] a_DataSize   The size fo the data.
 */
CGLVertexBuffer::CGLVertexBuffer(const void * a_VertexData, size_t a_DataSize, GLenum a_Usage) : m_Usage(a_Usage)
{
    glGenBuffers(1, &m_BufferId);
    glBindBuffer(GL_ARRAY_BUFFER, m_BufferId);
    glBufferData(GL_ARRAY_BUFFER, a_DataSize, a_VertexData, m_Usage);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CGLVertexBuffer::Update(const void * a_VertexData, size_t a_DataSize)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_BufferId);
    glBufferData(GL_ARRAY_BUFFER, a_DataSize, a_VertexData, m_Usage);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/**
 * \brief   Destructor. Performs the required cleanup.
 */
CGLVertexBuffer::~CGLVertexBuffer()
{
    glDeleteBuffers(1, &m_BufferId);
}

}
}