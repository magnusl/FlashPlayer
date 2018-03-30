#ifndef _CSHADERPROGRAM_H_
#define _CSHADERPROGRAM_H_

#include <memory>
#include <string>
#include <map>

namespace swf
{

namespace ogl
{

/**
 * OpenGL shader program
 */
class CShaderProgram
{
public:
    explicit CShaderProgram(int a_ProgramId);
    /**
     * \brief       Destructor.
     * \details     Performs the required resource cleanup.
     */
    virtual ~CShaderProgram();
    /**
     * \brief   Constructs a shader program from source files.
     */
    static std::shared_ptr<CShaderProgram> FromFiles(const char *, const char *);
    /**
     * \brief   Constructs a shader program from in memory sources.
    */
    static std::shared_ptr<CShaderProgram> FromSource(const char *, const char *);

    int GetProgram() const {return m_ProgramId;}
    int GetUniformLocation(const std::string & a_Name) const;

protected:
    CShaderProgram & operator=(const CShaderProgram &);
    CShaderProgram(const CShaderProgram &);

protected:

    std::map<std::string, int> m_Uniforms;
    int m_ProgramId;
};

}
}

#endif