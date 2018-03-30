#ifndef _CDEFLATESOURCE_H_
#define _CDEFLATESOURCE_H_

#include "binaryio.h"
#include <memory>

namespace avm2
{
namespace io
{

class CDeflateSource_Impl;
/**
 * Used to read deflated source files (ZLIB).
 */
class CDeflateSource : public CBinarySource
{
public:
    explicit CDeflateSource(std::shared_ptr<CFileSource> _source);
    ~CDeflateSource();

    virtual bool Eof() const;
    virtual bool Skip(size_t a_BytesToSkip);
    virtual bool Read(void * a_Dst, size_t a_Count);

protected:
    CDeflateSource(const CDeflateSource &);
    CDeflateSource & operator=(const CDeflateSource &);

private:
    CDeflateSource_Impl * impl;
};

} // namespace io
} // namespace avm2

#endif