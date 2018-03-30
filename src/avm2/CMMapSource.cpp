#include "CMMapSource.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace avm2
{

namespace io
{

class CMMapSource_impl
{
public:
    CMMapSource_impl(const wchar_t * a_FilePath);
    ~CMMapSource_impl();

    uint64_t GetSize() const        { return m_FileSize.QuadPart; }
    uint64_t FileOffset() const     { return m_Offset; }
    bool     Eof() const            { return !(m_Offset < GetSize()); }
    bool     Read(void * a_Dst, size_t a_Count);
    bool     Skip(size_t a_Count);

protected:
    HANDLE m_FileHandle;
    HANDLE m_FileMapping;
    const void * m_Data;
    LARGE_INTEGER m_FileSize;
    uint64_t m_Offset;
};

CMMapSource_impl::CMMapSource_impl(const wchar_t * a_FilePath) : m_Offset(0)
{
    m_FileHandle = CreateFile(a_FilePath,
        GENERIC_READ,
        FILE_SHARE_READ,    /**< allow other processes to read the same file */
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_READONLY | FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);
    if (!m_FileHandle) {
        throw std::runtime_error("Failed to open file for reading.");
    }

    if (!GetFileSizeEx(m_FileHandle, &m_FileSize)) {
        CloseHandle(m_FileHandle);
        throw std::runtime_error("Failed to get file size.");
    }

    m_FileMapping = CreateFileMapping(m_FileHandle, 
        NULL,
        PAGE_READONLY,
        0, 
        0,
        NULL);

    if (!m_FileMapping) {
        CloseHandle(m_FileHandle);
        throw std::runtime_error("Failed to create a file mapping.");
    }

    m_Data = MapViewOfFile(m_FileMapping, 
        FILE_MAP_READ,
        0,
        0,
        0); /**< map entire file */
        
    if (!m_Data) {
        CloseHandle(m_FileMapping);
        CloseHandle(m_FileHandle);
        throw std::runtime_error("Failed to map file.");
    }
}

/**
 * \brief   Destructor, performs the required cleanup.
 */
CMMapSource_impl::~CMMapSource_impl()
{
    UnmapViewOfFile(m_Data);
    CloseHandle(m_FileMapping);
    CloseHandle(m_FileHandle);
}

bool CMMapSource_impl::Read(void * a_Dst, size_t a_Count)
{
    uint64_t avail = GetSize() - FileOffset();
    if (avail < a_Count) {
        return false;
    }
    memcpy(a_Dst, ((const char *) m_Data) + m_Offset, a_Count);
    m_Offset += a_Count;
    return true;
}

bool CMMapSource_impl::Skip(size_t a_Count)
{
    uint64_t avail = GetSize() - FileOffset();
    if (avail < a_Count) {
        return false;
    }
    m_Offset += a_Count;
    return true;
}

CMMapSource::CMMapSource(const wchar_t * a_FilePath)
{
    impl = new (std::nothrow) CMMapSource_impl(a_FilePath);
}

CMMapSource::CMMapSource(const std::wstring & a_FilePath)
{
    impl = new (std::nothrow) CMMapSource_impl(a_FilePath.c_str());
}
    
CMMapSource::~CMMapSource()
{
    delete impl;
}

bool CMMapSource::Eof() const
{
    return impl->Eof();
}

bool CMMapSource::Read(void * a_Dst, size_t a_Count)
{
    return impl->Read(a_Dst, a_Count);
}

bool CMMapSource::Skip(size_t a_Count)
{
    return impl->Skip(a_Count);
}

uint64_t CMMapSource::GetSize() const
{
    return impl->GetSize();
}

uint64_t CMMapSource::FileOffset() const
{
    return impl->FileOffset();
}

}

}