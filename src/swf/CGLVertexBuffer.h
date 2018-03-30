#ifndef _CGLVERTEXBUFFER_H_
#define _CGLVERTEXBUFFER_H_

//#include <GL/glew.h>

namespace swf
{
namespace ogl
{

/**
 * \brief   OpenGL vertex buffer.
 */
class CGLVertexBuffer
{
public:
    CGLVertexBuffer(const void *, size_t, unsigned a_Usage);
    ~CGLVertexBuffer();

    void     Update(const void *, size_t);
    unsigned GetID() const {return m_BufferId;}

protected:

    CGLVertexBuffer(const CGLVertexBuffer &);
    CGLVertexBuffer & operator=(const CGLVertexBuffer &);

    unsigned m_BufferId;
    unsigned m_Usage;
};

}
}

#endif
