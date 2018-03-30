#include "binaryio.h"

namespace avm2
{
namespace io
{

CBinarySource::CBinarySource() : BitOffset(0), CachedValue(0)
{
}

CBinarySource::~CBinarySource()
{
}

uint32_t CBinarySource::GetU30()
{
    uint32_t dw = GetU8();
    if (!(dw & 0x80))           return dw;
    dw          = (dw & 0x7f) | (static_cast<uint32_t>(GetU8()) << 7);
    if (!(dw & 0x4000))         return dw;
    dw          = (dw & 0x3fff) | (static_cast<uint32_t>(GetU8()) << 14);
    if (!(dw & 0x200000))       return dw;
    dw          = (dw & 0x001FFFFF) | (static_cast<uint32_t>(GetU8()) << 21);
    if (!(dw & 0x10000000))     return dw;
    dw          = (dw & 0x0FFFFFFF) | (static_cast<uint32_t>(GetU8()) << 28);

    return dw;
}

uint8_t CBinarySource::GetU8()
{
    Align();
    uint8_t b;
    if (!Read(&b, sizeof(uint8_t))) {
        throw std::runtime_error("Failed to read byte from source.");
    }
    return b;
}

uint16_t CBinarySource::GetU16()
{
    Align();
    uint16_t w;
    if (!ReadBytes(&w, sizeof(uint16_t))) {
        throw std::runtime_error("Failed to read word from source.");
    }
    return w;
}

int16_t CBinarySource::GetS16()
{
    Align();
    int16_t w;
    if (!ReadBytes(&w, sizeof(int16_t))) {
        throw std::runtime_error("Failed to read word from source.");
    }
    return w;
}

uint32_t CBinarySource::GetU32()
{
    Align();
    uint32_t dw;
    if (!ReadBytes(&dw, sizeof(uint32_t))) {
        throw std::runtime_error("Failed to read dword from source.");
    }
    return dw;
}

int32_t CBinarySource::GetS24()
{
    Align();
    uint8_t v[3];
    v[0] = GetU8();
    v[1] = GetU8();
    v[2] = GetU8();

    int32_t s24 = (((unsigned) v[2]) << 16) | (((unsigned) v[1]) << 8) | v[0];
    if (s24 & 0x800000) {
        return (s24 | 0xff000000);
    } else {
        return s24;
    }
}

int32_t CBinarySource::GetS32()
{
    Align();
    int32_t dw;
    if (!ReadBytes(&dw, sizeof(int32_t))) {
        throw std::runtime_error("Failed to read dword from source.");
    }
    return dw;
}

double CBinarySource::GetDouble()
{
    Align();
    double d;
    if (!Read(&d, sizeof(d))) {
        throw std::runtime_error("failed to read double from source.");
    }
    return d;
}

float CBinarySource::GetFloat()
{
    Align();
    float f;
    if (!Read(&f, sizeof(f))) {
        throw std::runtime_error("Failed to read float from source.");
    }
    return f;
}

std::string CBinarySource::GetString(void)
{
    std::string str;
    uint8_t c = GetU8();
    while(c != 0) {
        str += (char) c;
        c = GetU8();
    }
    return str;
}

/**
 * \brief Reads a variable length encoded signed 16.16 fixed point number.
 */
int32_t CBinarySource::GetFixed16(int a_NumBits)
{
    uint32_t w = GetBits(a_NumBits);
    if (w & (1 << (a_NumBits - 1))) {
        return (int32_t) (w | (0xffffffff << a_NumBits));   // sign extend.
    }
    return (int32_t) w;
}

int16_t CBinarySource::GetFixed8()
{
    int16_t w = GetSignedBits(16);
    return w;
}

bool CBinarySource::ReadBytes(void * src, int32_t count)
{
    if (count < 0 || count > 4) {
        return false;
    }
    if (!BitOffset) {
        /** aligned in the byte stream */
        return Read(src, count);
    } else {
        /** not aligned in the byte stream */
        for(int32_t i = 0; i < count; i++) {
            /** read one byte at the time for now, this can be optimized later for platforms that
                allows unaligned accesses */
            *((uint8_t *) src) = static_cast<uint8_t>(GetBits(8));
        }
    }
    return true;
}

/**
 * Reads up to 32 bits from the bitstream and returns them as the lower bits of a uint32_t.
 */
uint32_t CBinarySource::GetBits(size_t count) 
{
    uint32_t dw = 0;
    while(count > 0) 
    {
        if (BitOffset) {
            /** the number of bits to read from the current cached value */
            size_t rc = count > BitOffset ? BitOffset : count;
            size_t sc = BitOffset - rc;
            dw = (dw << rc) | ((CachedValue >> sc) & (0xff >> (8 - rc)));
            count -= rc;
            BitOffset -= rc;
        } else {
            if (!Read(&CachedValue, sizeof(uint8_t))) {
                throw std::runtime_error("Failed to read data from source.");
            }
            BitOffset = 8;
        }
    }
    return dw;
}

int32_t CBinarySource::GetSignedBits(size_t count)
{
    uint32_t dw = GetBits(count);
    if (count > 0) {
        if (dw & (1 << (count-1))) {
            uint32_t mask = (0xffffffff << count);
            return (int32_t) (dw | mask);
        }
    }
    return dw;
}

void CBinarySource::Align()
{
    /** ignore any remaining bits in the cached value */
    BitOffset = 0;
}

void CBinarySource::AssureAlignment()
{
    if (BitOffset) {
        throw std::runtime_error("Not byte aligned.");
    }
}

}
}