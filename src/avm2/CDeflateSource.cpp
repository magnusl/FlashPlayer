#include "CDeflateSource.h"
#include <vector>
#include "miniz.c"

#define DEFLATE_BUFFER_SIZE (64 * 1024) 

namespace avm2
{
namespace io
{

class CDeflateSource_Impl
{
public:
    CDeflateSource_Impl(std::shared_ptr<CFileSource>);
    ~CDeflateSource_Impl();

    bool Read(void * a_Dst, size_t a_Count);
    bool Eof();
    bool Skip(size_t a_BytesToSkip);
    bool FillBuffer();

    std::shared_ptr<CFileSource>    compressedFile;
    mz_stream                       stream;
    std::vector<uint8_t>            outBuffer;
    std::vector<uint8_t>            inputBuffer;
    size_t                          availOut;
    size_t                          offset;
};

CDeflateSource::CDeflateSource(std::shared_ptr<CFileSource> _compressedSource)
{
    impl = new (std::nothrow) CDeflateSource_Impl(_compressedSource);
}

CDeflateSource::~CDeflateSource()
{
    delete impl;
}

bool CDeflateSource::Read(void * a_Dst, size_t a_Count)
{
    return impl->Read(a_Dst, a_Count);
}

bool CDeflateSource::Eof() const
{
    return impl->Eof();
}

bool CDeflateSource::Skip(size_t a_BytesToSkip)
{
    return impl->Skip(a_BytesToSkip);
}

CDeflateSource_Impl::CDeflateSource_Impl(std::shared_ptr<CFileSource> a_CompressedSource) :
    availOut(0),
    offset(0)
{
    compressedFile = a_CompressedSource;
    outBuffer.resize(DEFLATE_BUFFER_SIZE);
    inputBuffer.resize(DEFLATE_BUFFER_SIZE);

    memset(&stream, 0, sizeof(mz_stream));

    if (inflateInit2(&stream, MZ_DEFAULT_WINDOW_BITS)) {
        throw std::runtime_error("inflateInit failed.");
    }
    stream.next_out  = &outBuffer[0];
    stream.avail_out = DEFLATE_BUFFER_SIZE;
}

CDeflateSource_Impl::~CDeflateSource_Impl()
{
    inflateEnd(&stream);
}

bool CDeflateSource_Impl::Read(void * _dst, size_t _count)
{
    if (Eof()) {
        return false;
    }
    uint8_t * pDst = (uint8_t *) _dst;
    while(_count) {
        if (offset < availOut) { /**< we have data in the buffer, so copy it */
            size_t n = std::min(_count, availOut - offset);
            memcpy(pDst, &outBuffer[offset], n);
            _count  -= n;
            offset += n;
            pDst     += n;
        } else { 
            /** we don't have any data in the buffer, so read some more from the compressed source */
            if (!FillBuffer()) {
                return false;
            }
        }
    }
    return true;
}

bool CDeflateSource_Impl::Skip(size_t _bytesToSkip)
{
    if (Eof()) {
        return false;
    }
    while(_bytesToSkip) {
        if (offset < availOut) { /**< we have data in the buffer, so copy it */
            size_t n = std::min(_bytesToSkip, availOut - offset);
            _bytesToSkip    -= n;
            offset      += n;
        } else { 
            /** we don't have any data in the buffer, so read some more from the compressed source */
            if (!FillBuffer()) {
                return false;
            }
        }
    }
    return true;
}

bool CDeflateSource_Impl::Eof()
{
    if (offset < availOut) { /**< we still have data in the buffer */
        return false;
    } else { /**< check if there is more data in the compressed file */
        return compressedFile->Eof();
    }
}

bool CDeflateSource_Impl::FillBuffer()
{
    if (compressedFile->Eof()) { /**< cannot read any more data */
        return false;
    }

    if (!stream.avail_in) 
    {
        // read data into the input buffer.
        uint64_t avail = compressedFile->GetSize() - compressedFile->FileOffset();
        size_t n       = (size_t) std::min(avail, (uint64_t) DEFLATE_BUFFER_SIZE);
        if (!compressedFile->Read(&inputBuffer[0], n)) {
            return false;
        }
        stream.next_in  = &inputBuffer[0];  /**< source buffer */
        stream.avail_in = n;                    /**< number of available bytes */
    }

    int status = inflate(&stream, Z_SYNC_FLUSH);
    if ((status == Z_STREAM_END) || (!stream.avail_out))
    {
        offset              = 0;
        availOut            = stream.avail_out;
        stream.next_out     = &inputBuffer[0];
        stream.avail_out    = DEFLATE_BUFFER_SIZE;
    } else if (status == Z_STREAM_END) {
        return false;
    } else {
        return false;
    }
    return true;
}

} // namespace io
} // namespace avm2
