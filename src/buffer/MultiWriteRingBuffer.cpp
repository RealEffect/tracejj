#include "buffer/MultiWriteRingBuffer.h"
#include <cstring>

MultiWriteRingBuffer::MultiWriteRingBuffer(uint32_t uBytes)
    : m_pData(new int8_t[uBytes])
    , m_uCapOfBytes(uBytes)
    , m_uBeginIndex(0U)
    , m_uEndIndex(0U)
    , m_uDataOfBytes(0U)
{
}

MultiWriteRingBuffer::~MultiWriteRingBuffer()
{
    m_uCapOfBytes = 0;
    delete[] m_pData;
    m_pData = nullptr;
}

bool MultiWriteRingBuffer::Write(const int8_t* pData, uint32_t uBytes)
{
    if (m_pData == nullptr || (m_uCapOfBytes - m_uDataOfBytes.load()) < uBytes)
    {
        return false;
    }
    if (pData == nullptr || uBytes == 0U)
    {
        return true;
    }
    auto uEndIndex = m_uEndIndex.load();
    uint32_t uWriteableIndex = m_uWriteableIndex.load();
    auto uNewWriteableIndex = uWriteableIndex;
    while (true)
    {
        uNewWriteableIndex = uWriteableIndex + uBytes;
        while (uNewWriteableIndex >= m_uCapOfBytes)
        {
            uNewWriteableIndex -= m_uCapOfBytes;
        }
        if (m_uWriteableIndex.compare_exchange_strong(uWriteableIndex, uNewWriteableIndex))
        {
            break;
        }
        if ((m_uCapOfBytes - m_uDataOfBytes.load()) < uBytes)
        {
            return false;
        }
    }
    auto pWriteBuffer = m_pData + uWriteableIndex;
    if (uWriteableIndex + uBytes <= m_uCapOfBytes)
    {
        memcpy(pWriteBuffer, pData, uBytes);
    }
    else
    {
        const uint32_t uSilce0Bytes = m_uCapOfBytes - uWriteableIndex;
        memcpy(pWriteBuffer, pData, uSilce0Bytes);
        pData += uSilce0Bytes;
        // pWriteBuffer = m_pData;
        memcpy(m_pData, pData, uBytes - uSilce0Bytes);
    }
    while (true)
    {
        if (uWriteableIndex == m_uEndIndex.load())
        {
            m_uEndIndex.store(uNewWriteableIndex);
            m_uDataOfBytes += uBytes;
            break;
        }
    }
    return true;
}

MultiWriteRingBuffer::Block MultiWriteRingBuffer::Peek() const
{
    // Single thread
    MultiWriteRingBuffer::Block b;
    const uint32_t uBytes = m_uDataOfBytes;
    const uint32_t uBeginIndex = m_uBeginIndex;
    b.pSlice0 = m_pData + uBeginIndex;
    if ((uBeginIndex + uBytes) <= m_uCapOfBytes)
    {
        b.uSlice0Bytes = uBytes;
    }
    else
    {
        b.uSlice0Bytes = m_uCapOfBytes - uBeginIndex;
        b.pSlice1 = m_pData;
        b.uSlice1Bytes = uBytes - b.uSlice0Bytes;
    }
    return b;
}

bool MultiWriteRingBuffer::Readed(const Block& b)
{
    // Single thread
    const uint32_t uBytes = b.uSlice0Bytes + b.uSlice1Bytes;
    if (uBytes > m_uDataOfBytes.load())
    {
        return false;
    }
    auto uBeginIndex = m_uBeginIndex.load();
    uBeginIndex += uBytes;
    while (uBeginIndex >= m_uCapOfBytes)
    {
        uBeginIndex -= m_uCapOfBytes;
    }
    m_uDataOfBytes -= uBytes;
    m_uBeginIndex = uBeginIndex;
    return true;
}
