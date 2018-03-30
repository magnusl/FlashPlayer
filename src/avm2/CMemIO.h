#ifndef _MEMIO_H_
#define _MEMIO_H_

#include "binaryio.h"
#include <vector>

namespace avm2
{

namespace io
{

class CMemIO : public CBinarySource
{
public:
    CMemIO(const std::vector<uint8_t> &);
    size_t GetSize() const;
    size_t GetOffset() const;

    virtual bool Eof() const;
    virtual bool Skip(size_t _bytesToSkip);
    virtual bool Read(void * _dst, size_t _count);
protected:
    const std::vector<uint8_t> & data;
    size_t                       offset;
};

} // namespace io

} // namespace avm2

#endif