//
// Created by Tomek Jaworski on 19/02/2021.
//

#ifndef UNTITLED_BINARYQUEUE_H
#define UNTITLED_BINARYQUEUE_H

#include <cstdint>
#include <memory>

class BinaryQueue {
private:
    uint8_t* data;
    uint32_t capacity;
    uint32_t position;

public:
    BinaryQueue(uint32_t capacity = 1024);
    BinaryQueue(const BinaryQueue& copy);
    ~BinaryQueue();
    BinaryQueue& operator=(const BinaryQueue& copy);



    void EnqueueBytes(const void* buffer, uint32_t offset, uint32_t count);
    void DequeueBytes(uint32_t count);
    void Clear(void);

public:
    uint32_t GetCapacity(void) const { return this->capacity; }
    uint32_t GetLength(void) const { return this->position; }

    bool HasAtLeast(uint32_t length) const {
        return this->position >= length;
    }

    template <typename T = void> const T* GetDataPointer(void) const { return reinterpret_cast<const T*>(this->data); }
    template <typename T = void> T* GetDataPointer(void) { return reinterpret_cast<T*>(this->data); }

    template <typename T = void>
    T PeekValue(uint32_t position) {
        const T* ptr = reinterpret_cast<const T*>(GetDataPointer<uint8_t>() + position);
        return *ptr;
    }
};




#endif //UNTITLED_BINARYQUEUE_H
