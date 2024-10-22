#include "MultiWriteRingBuffer.h"
#include <cstring>

namespace tracejj
{

MultiWriteRingBuffer::MultiWriteRingBuffer(size_t szBytes)
    : m_pData(new int8_t[szBytes])
    , m_szCapOfBytes(szBytes)
    , m_uBeginIndex(0u)
    , m_uWriteableIndex(0u)
    , m_uEndIndex(0u)
    , m_uDataOfBytes(0u)
{
}

MultiWriteRingBuffer::~MultiWriteRingBuffer()
{
    m_szCapOfBytes = 0;
    delete[] m_pData;
    m_pData = nullptr;
}

bool MultiWriteRingBuffer::Write(const int8_t* pData, size_t szBytes)
{
    if (m_pData == nullptr || (m_szCapOfBytes - m_uDataOfBytes.load()) < szBytes)
    {
        return false;
    }
    if (pData == nullptr || szBytes == 0U)
    {
        return true;
    }
    size_t uWriteableIndex = m_uWriteableIndex.load();
    size_t uNewWriteableIndex = 0u;
    while (true)
    {
        uNewWriteableIndex = uWriteableIndex + szBytes;
        while (uNewWriteableIndex >= m_szCapOfBytes)
        {
            uNewWriteableIndex -= m_szCapOfBytes;
        }
        if (m_uWriteableIndex.compare_exchange_strong(uWriteableIndex, uNewWriteableIndex))
        {
            break;
        }
        if ((m_szCapOfBytes - m_uDataOfBytes.load()) < szBytes)
        {
            return false;
        }
    }
    auto pWriteBuffer = m_pData + uWriteableIndex;
    if ((szBytes + uWriteableIndex) <= m_szCapOfBytes)
    {
        memcpy(pWriteBuffer, pData, szBytes);
    }
    else
    {
        const size_t szSilce0Bytes = m_szCapOfBytes - uWriteableIndex;
        memcpy(pWriteBuffer, pData, szSilce0Bytes);
        pData += szSilce0Bytes;
        memcpy(m_pData, pData, szBytes - szSilce0Bytes);
    }
    while (true)
    {
        if (uWriteableIndex == m_uEndIndex.load())
        {
            m_uEndIndex.store(uNewWriteableIndex);
            m_uDataOfBytes += static_cast<uint32_t>(szBytes);
            break;
        }
    }
    return true;
}

MultiWriteRingBuffer::Block MultiWriteRingBuffer::Peek() const
{
    // Single thread
    MultiWriteRingBuffer::Block b;
    const size_t szBytes = m_uDataOfBytes;
    const size_t szBeginIndex = m_uBeginIndex;
    b.pSlice0 = m_pData + szBeginIndex;
    if ((szBeginIndex + szBytes) <= m_szCapOfBytes)
    {
        b.szSlice0 = szBytes;
    }
    else
    {
        b.szSlice0 = m_szCapOfBytes - szBeginIndex;
        b.pSlice1 = m_pData;
        b.szSlice1 = szBytes - b.szSlice0;
    }
    return b;
}

bool MultiWriteRingBuffer::Readed(const Block& b)
{
    // Single thread
    const size_t szBytes = b.szSlice0 + b.szSlice1;
    if (szBytes > m_uDataOfBytes.load())
    {
        return false;
    }
    auto uBeginIndex = m_uBeginIndex.load();
    uBeginIndex += szBytes;
    while (uBeginIndex >= m_szCapOfBytes)
    {
        uBeginIndex -= m_szCapOfBytes;
    }
    m_uDataOfBytes -= szBytes;
    m_uBeginIndex = uBeginIndex;
    return true;
}

}  // namespace tracejj
