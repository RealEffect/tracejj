#pragma once

#include <atomic>
#include <cstdint>
#include <cstddef>

namespace tracejj
{
class MultiWriteRingBuffer
{
public:
    struct Block
    {
        const int8_t* pSlice0 = nullptr;
        size_t szSlice0 = 0ull;
        const int8_t* pSlice1 = nullptr;
        size_t szSlice1 = 0ull;
    };

public:
    explicit MultiWriteRingBuffer(size_t szBytes);
    virtual ~MultiWriteRingBuffer();
    MultiWriteRingBuffer() noexcept = delete;
    MultiWriteRingBuffer(const MultiWriteRingBuffer&) noexcept = delete;
    MultiWriteRingBuffer(MultiWriteRingBuffer&&) noexcept = delete;
    MultiWriteRingBuffer& operator=(const MultiWriteRingBuffer&) noexcept = delete;
    MultiWriteRingBuffer& operator=(MultiWriteRingBuffer&&) noexcept = delete;

public:
    bool Write(const int8_t* pData, size_t szBytes);
    Block Peek() const;
    bool Readed(const Block& b);
    const int8_t* Data() const
    {
        return m_pData + m_uBeginIndex.load();
    }
    size_t Bytes() const
    {
        return m_uDataOfBytes.load();
    }
    bool Empty() const
    {
        return m_uDataOfBytes.load() == 0U;
    }

private:
    int8_t* m_pData;
    size_t m_szCapOfBytes;
    std::atomic<size_t> m_uBeginIndex;
    std::atomic<size_t> m_uWriteableIndex;
    std::atomic<size_t> m_uEndIndex;
    std::atomic<size_t> m_uDataOfBytes;
};
}  // namespace tracejj