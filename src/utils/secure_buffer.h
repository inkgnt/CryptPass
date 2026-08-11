#pragma once

#include "lock_limit.h"
#include "widget_helpers.h"

#include <sodium.h>

#include <cstdlib>
#include <stdexcept>
#include <cstdint>
#include <QDebug>
#include <algorithm>
#include <cstring>


//todo debug metrics
class SecureBuffer
{
public:

    SecureBuffer() = default;

    explicit SecureBuffer(std::size_t size) {
        if (size > 0) {
            reallocate(size);
            size_ = size;
        }
    }

    ~SecureBuffer() {
        destroy();
    }

    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer& operator=(const SecureBuffer&) = delete;

    SecureBuffer(SecureBuffer&& other) noexcept
        :data_(other.data_), size_(other.size_), capacity_(other.capacity_)
    {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    };

    SecureBuffer& operator=(SecureBuffer&& other) noexcept
    {
        if(this != &other) {
            destroy();

            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;

            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    std::uint8_t* data() noexcept { return data_; };
    const std::uint8_t* data() const noexcept { return data_; };
    std::size_t size() const noexcept { return size_; };
    std::size_t capacity() const noexcept { return capacity_; };
    bool empty() const noexcept {return !size_;  };

    void reserve(std::size_t new_capacity) {
        if (new_capacity <= capacity_)
            return;
        reallocate(new_capacity);
    }

    void resize(std::size_t new_size) {

        if (new_size > capacity_) {
            std::size_t new_capacity = std::max(new_size, capacity_ + capacity_ / 2);
            reserve(new_capacity);
        }
        else if (new_size < size_) {
            sodium_memzero(data_ + new_size, size_ - new_size);
        }
        size_ = new_size;
    }

    bool contains(const SecureBuffer& other, Qt::CaseSensitivity cs) const {
        return contains(other.data(), other.size(), cs);
    }


private:
    inline static std::atomic<std::size_t> global_memory_used{0};
    inline static std::atomic<std::size_t> global_counter_free{0};
    inline static std::atomic<std::size_t> global_counter_malloc{0};

    std::uint8_t* data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;

    void destroy() {
        if (data_) {
            qDebug() << "sodium_free triggered" <<  global_counter_free++;
            sodium_free(data_);

            global_memory_used -= capacity_;
            qDebug() << "total memory used after sodium_free:" << global_memory_used ;
        }
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    void reallocate(std::size_t new_capacity) {
        std::uint8_t* new_data = (std::uint8_t*)sodium_malloc(new_capacity);
        qDebug() << "sodium_malloc" << global_counter_malloc++ << "triggered, size:" << new_capacity;

        global_memory_used += new_capacity;
        qDebug() << "total memory used after sodium_malloc:" << global_memory_used ;

        if (!new_data) {
            throw std::bad_alloc();
        }

        sodium_memzero(new_data, new_capacity);

        if (sodium_mlock(new_data, new_capacity) == -1) {
#ifdef _WIN32
            if (!increaseWindowsLockLimit(new_capacity) || sodium_mlock(new_data, new_capacity) == -1) {
                sodium_free(new_data);
                throw std::runtime_error("Critical failure! Operating System refused to lock memory pages.");
            }
#else
            sodium_free(new_data);
            throw std::runtime_error("Critical failure! Operating System refused to lock memory pages.");
#endif
        }

        if (data_) {
            std::size_t copy_len = std::min(size_, new_capacity);
            if (copy_len > 0) {
                std::memcpy(new_data, data_, copy_len);
            }

            sodium_free(data_);

            qDebug() << "sodium_free" << global_counter_free++ << "triggered, size:" << new_capacity;

            global_memory_used -= capacity_;
            qDebug() << "total memory used after sodium_free:" << global_memory_used ;
        }

        data_ = new_data;
        capacity_ = new_capacity;
    }


    //TODO


    bool contains(const std::uint8_t* pData, std::size_t pSize, Qt::CaseSensitivity cs) const {
        if (pSize == 0) return true;
        if (pSize > size_) return false;

        if (cs == Qt::CaseSensitive) {
            return std::search(data_, data_ + size_, pData, pData + pSize) != (data_ + size_);
        } else {
            return utf8SearchInsensitive(pData, pSize);
        }
    }

    static std::uint32_t toLowerUnicode(std::uint32_t cp) {
        if (cp >= 0x0041 && cp <= 0x005A) return cp + 0x20;

        if (cp >= 0x0410 && cp <= 0x042F) return cp + 0x20;

        if (cp == 0x0401) return 0x0451;

        if ((cp >= 0x00C0 && cp <= 0x00D6) || (cp >= 0x00D8 && cp <= 0x00DE)) return cp + 0x20;

        return cp;
    }

    bool utf8SearchInsensitive(const std::uint8_t* pData, std::size_t pSize) const {
        std::vector<std::uint32_t> patternCP;
        const std::uint8_t* pPtr = pData;
        const std::uint8_t* pEnd = pData + pSize;
        while (pPtr < pEnd) {
            patternCP.push_back(toLowerUnicode(nextUtf8Codepoint(pPtr, pEnd)));
        }

        if (patternCP.empty()) return true;

        const std::uint8_t* currentStart = data_;
        const std::uint8_t* totalEnd = data_ + size_;

        while (currentStart < totalEnd) {
            const std::uint8_t* searchPtr = currentStart;
            bool match = true;

            for (std::uint32_t pCP : patternCP) {
                if (searchPtr >= totalEnd) {
                    match = false;
                    break;
                }
                std::uint32_t bufferCP = toLowerUnicode(nextUtf8Codepoint(searchPtr, totalEnd));
                if (bufferCP != pCP) {
                    match = false;
                    break;
                }
            }

            if (match) return true;

            nextUtf8Codepoint(currentStart, totalEnd);
        }

        return false;
    }

    static inline std::uint8_t toLowerAscii(std::uint8_t c) {
        if (c >= 'A' && c <= 'Z') return c + 32;
        return c;
    }

};
