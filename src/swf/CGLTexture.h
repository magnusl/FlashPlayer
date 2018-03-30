#ifndef _CGLTEXTURE_H_
#define _CGLTEXTURE_H_

namespace swf
{
namespace ogl
{

/**
 * OpenGL texture.
 */
class CGLTexture
{
public:
    typedef enum {
        eRGBA_TEXTURE,  /**< 32-bit RGBA texture */
        eRGB_TEXTURE,   /**< 24-bit RGB texture */
        eFLOAT_TEXTURE, /**< Single precision floating point texture */
    } Format_t;

    virtual ~CGLTexture();
    unsigned GetID() const;

protected:
    unsigned    m_Id;
};

/**
 * \brief   1D OpenGL texture.
 */
class CGLTexture1D : public CGLTexture
{
public:
    /**
     * \brief   Constructor. Initializes the 1D texture based on the
     *          arguments.
     */
    CGLTexture1D(Format_t a_Format, void * a_Data, size_t a_Width);
    /**
     * \brief   Destructor.
     */
    virtual ~CGLTexture1D();
};

/**
 * \brief   2D OpenGL texture.
 */
class CGLTexture2D : public CGLTexture
{
public:
    CGLTexture2D(Format_t a_Format, void * a_Data, size_t a_Width, size_t a_Height);
    virtual ~CGLTexture2D();
};

}
}

#endif
