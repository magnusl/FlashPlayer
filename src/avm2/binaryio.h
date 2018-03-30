#ifndef _BINARYIO_H_
#define _BINARYIO_H_

#include <stdint.h>
#include <string>

namespace avm2
{
namespace io
{

/**
 * Deserializes data from a binary source.
 */
class CBinarySource
{
public:
    CBinarySource();
    virtual ~CBinarySource();

    uint32_t        GetBits(size_t count);
    int32_t         GetSignedBits(size_t count) ;

    uint8_t         GetU8();
    uint16_t        GetU16();
    uint32_t        GetU32();
    uint32_t        GetU30();
    int32_t         GetS24();
    int16_t         GetS16();
    int32_t         GetS32();
    int32_t         GetFixed16(int a_NumBits);
    int16_t         GetFixed8();
    double          GetDouble();
    float           GetFloat();
    std::string     GetString();
    void            Align();
    void            AssureAlignment();

    virtual bool Eof() const = 0;
    virtual bool Skip(size_t _bytesToSkip) = 0;
    virtual bool Read(void * _dst, size_t _count) = 0;

protected:

    bool ReadBytes(void * src, int32_t count);

    size_t          BitOffset; 
    uint8_t         CachedValue;
};

class CFileSource : public CBinarySource
{
public:
    virtual uint64_t GetSize() const    = 0;
    virtual uint64_t FileOffset() const = 0;
};

}
}

#endif