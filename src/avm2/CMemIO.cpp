#include "CMemIO.h"

namespace avm2
{
namespace io
{

CMemIO::CMemIO(const std::vector<uint8_t> & _source) : 
    data(_source), offset(0)
{
}

size_t CMemIO::GetSize() const
{
    return data.size();
}

size_t CMemIO::GetOffset() const
{
    return offset;
}

bool CMemIO::Eof() const
{
    return offset > GetSize();
}

bool CMemIO::Skip(size_t _bytesToSkip)
{
    if ((offset + _bytesToSkip) > GetSize()) {
        return false;
    }
    offset += _bytesToSkip;
    return true;
}

bool CMemIO::Read(void * _dst, size_t _count)
{
    if ((offset + _count) > GetSize()) {
        return false;
    }
    memcpy(_dst, &data[offset], _count);
    offset += _count;
    return true;
}

}
}
