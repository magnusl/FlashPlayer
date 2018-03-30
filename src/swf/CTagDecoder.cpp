#include "CTagDecoder.h"

namespace swf
{

/**
 * 
 * \param       source      The IoSource object to decode the SWF tag from.
 */
CTagDecoder::CTagDecoder(avm2::io::CBinarySource & source) : m_Source(source), m_Offset(0)
{
    uint16_t TagCodeAndLength, length;

    TagCodeAndLength = source.GetU16();
    /** The tag code is encoded in the upper 10 bits */
    m_TagCode = (TagCodeAndLength >> 6);
    length = TagCodeAndLength & 0x3f;
    if (length != 0x3f) {
        m_TagLength = static_cast<int32_t>(length);
    } else {
        m_TagLength = source.GetU32();
    }
}

CTagDecoder::~CTagDecoder()
{
    Skip(m_TagLength - m_Offset);
}

bool CTagDecoder::Skip(size_t offset)
{
    if ((offset + m_Offset) > m_TagLength) {
        return false;
    }
    m_Offset += offset;
    return m_Source.Skip(offset);
}

void CTagDecoder::SetPosition(size_t a_Offset)
{
    if (m_Offset > a_Offset) {
        throw std::runtime_error("Cannot rewind.");
    } else if (m_Offset == a_Offset) {
        BitOffset = 0;
    } else {
        BitOffset = 0;
        size_t diff = a_Offset - m_Offset;
        if (!Skip(diff)) {
            throw std::runtime_error("Can't jump to specified position.");
        }
    }
}

/**
 * Reads data from the tag.
 *
 * \param       dst         The destination buffer.
 * \param       count       The number of bytes to read from the tag.
 *
 * \return      true        It was possible to read the data.
 * \return      false       It was not possible to read the data.
 */
bool CTagDecoder::Read(void * dst, size_t count)
{
    if ((count + m_Offset) > m_TagLength) {
        return false;
    }
    m_Offset += count;
    return m_Source.Read(dst, count);
}

}