#pragma once

#include <atomic>
#include <cstdint>

class MultiWriteRingBuffer
{
    struct Block
    {
        const int8_t* pSlice0 = nullptr;
        const int8_t* pSlice1 = nullptr;
        uint32_t uSlice0Bytes = 0U;
        uint32_t uSlice1Bytes = 0U;
    };

public:
    MultiWriteRingBuffer() noexcept = delete;
    MultiWriteRingBuffer(const MultiWriteRingBuffer&) noexcept = delete;
    MultiWriteRingBuffer(MultiWriteRingBuffer&&) noexcept = delete;
    explicit MultiWriteRingBuffer(uint32_t uBytes);
    virtual ~MultiWriteRingBuffer();

public:
    MultiWriteRingBuffer& operator=(const MultiWriteRingBuffer&) noexcept = delete;
    MultiWriteRingBuffer& operator=(MultiWriteRingBuffer&&) noexcept = delete;

public:
    bool Write(const int8_t* pData, uint32_t uBytes);
    Block Peek() const;
    bool Readed(const Block& b);
    inline const int8_t* Data() const
    {
        return m_pData + m_uBeginIndex.load();
    }
    inline uint32_t Bytes() const
    {
        return m_uDataOfBytes.load();
    }
    inline bool Empty() const
    {
        return m_uDataOfBytes.load() == 0U;
    }

private:
    int8_t* m_pData;
    uint32_t m_uCapOfBytes;
    std::atomic_uint32_t m_uBeginIndex;
    std::atomic_uint32_t m_uWriteableIndex;
    std::atomic_uint32_t m_uEndIndex;
    std::atomic_uint32_t m_uDataOfBytes;
};