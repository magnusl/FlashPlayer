#ifndef _CTAGDECODER_H_
#define _CTAGDECODER_H_

#include <avm2\binaryio.h>

namespace swf
{

/**
 * Decodes a SWF tag.
 */
class CTagDecoder : public avm2::io::CBinarySource
{
public:
    explicit CTagDecoder(avm2::io::CBinarySource &);
    ~CTagDecoder();

    uint32_t        TagLength() const   {return m_TagLength;}
    uint16_t        TagCode() const     {return m_TagCode;}
    uint32_t        TagOffset() const   {return m_Offset;}

    virtual void    SetPosition(size_t a_Offset);
    virtual bool    Eof() const         {return m_Offset >= m_TagLength;}
    virtual bool    Skip(size_t);
    
    bool Read(void *, size_t);

protected: // inherited pure virtual methods
    CTagDecoder & operator=(const CTagDecoder &);
    CTagDecoder(const CTagDecoder &);

protected:
    avm2::io::CBinarySource &   m_Source;
    uint32_t    m_TagLength;
    uint16_t    m_TagCode;
    uint32_t    m_Offset;
};

}

#endif