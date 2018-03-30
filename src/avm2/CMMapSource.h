#ifndef _CMMAPSOURCE_H_
#define _CMMAPSOURCE_H_

#include "binaryio.h"

namespace avm2
{

namespace io
{

class CMMapSource_impl;

/**
 * Source which uses a memory mapped file
 */
class CMMapSource : public CFileSource
{
public:
    CMMapSource(const wchar_t * a_FilePath);
    CMMapSource(const std::wstring & a_FilePath);
    virtual ~CMMapSource();

    // io::Source
    virtual bool Eof() const;
    virtual bool Skip(size_t a_BytesToSkip);
    virtual bool Read(void * a_Dst, size_t a_Count);

    // io::FileSource
    virtual uint64_t GetSize() const;
    virtual uint64_t FileOffset() const;

protected:
    CMMapSource_impl * impl;
};

}

}

#endif