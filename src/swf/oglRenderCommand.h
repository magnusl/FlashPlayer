#ifndef _OGLRENDERCOMMAND_H_
#define _OGLRENDERCOMMAND_H_

#include <stdint.h>
//#include <gl\glew.h>
//#include <GL\GL.h>

namespace swf
{
namespace ogl
{

/**
 * \brief   Represents a single command which can either change the state of the renderer,
 *      or emit actual verticies to render. Contains references to OpenGL resources that are owned by
 *      other components, so care must be taken if the command list is saved for future used.
 */
struct RenderingCommand {
    typedef enum {
        RC_USE_PROGRAM,             /**< Selects a shader program, invalidates all other state and is very costly */
        RC_BIND_TEXTURE,            /**< Binds a texture to a specific sampler */
        RC_BIND_VERTICIES,          /**< bind the verticies */
        RC_BIND_INDICIES,           /**< bind the buffer containing the indicies */
        RC_BIND_TEXCOORDS,          /**< bind the buffer containing the texcoords */
        RC_DRAW_ELEMENTS,           /**< Draw triangles/lines/points using indexed rendering */
        RC_SET_UNIFORM_FLOAT,       /**< Sets a uniform value */
        RC_ENABLE_STENCIL_TEST,     /**< enables stencil testing */
        RC_STENCIL_TEST,            /**< objects drawn after this command is tested against the stencil buffer */
        RC_DISABLE_STENCIL_TEST,    /**< disables stencil testing */
        RC_DRAW_TEXT,
    } CmdType;

    RenderingCommand()
    {}

    RenderingCommand(CmdType a_Type) : Type(a_Type)
    {}

    CmdType Type;

    union {
        uint32_t    ProgramID;  /**< Shader program (RC_USE_PROGRAM) */
        struct {
            unsigned    Mode;
            int         Count;
            unsigned    Type;
            const void * Indicies;
        } elements;         /**< contains the information required for a glDrawElements call */
        struct {
            unsigned int Location;
            float   Values[16];
            uint8_t Count;
        } uniform;
        struct {
            unsigned int BufferID;
            unsigned int Location;
        } verticies;
        struct {
            unsigned int BufferID;
        } indicies;
        struct {
            int          Sampler;
            unsigned int TextureID;
        } texture;
        struct {
            int          Location;
            unsigned int BufferID;
        } texcoords;
        struct {
            size_t      StringIndex;
            int         FontId;
            float       Size;
            float       x;
            float       y;
        } text;
    } u;
};

}
}
#endif
