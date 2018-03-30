namespace swf
{
namespace ogl
{
    /**
     * OpenGL index buffer
     */
    class CGLIndexBuffer
    {
    public:
        CGLIndexBuffer(const void *, size_t);
        ~CGLIndexBuffer();

        unsigned GetID() const {return m_BufferId;}
    protected:
        unsigned m_BufferId;
    };
}
}