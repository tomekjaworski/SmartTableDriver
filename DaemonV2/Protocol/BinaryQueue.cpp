//
// Created by Tomek Jaworski on 19/02/2021.
//

#include <stdexcept>
#include <cstring>
#include "BinaryQueue.hpp"

BinaryQueue::BinaryQueue(uint32_t capacity) {
    this->capacity = capacity;
    this->data = new uint8_t[capacity];
    this->position = 0;
}

BinaryQueue::BinaryQueue(const BinaryQueue& copy) {
    //
    this->capacity = copy.capacity;
    this->data = new uint8_t[copy.capacity];
    std::memcpy(this->data, copy.data, copy.position);
    this->position = copy.position;
}

BinaryQueue::~BinaryQueue() {
    delete[] this->data;
}

BinaryQueue& BinaryQueue::operator=(const BinaryQueue& copy) {
    this->capacity = copy.capacity;
    this->data = new uint8_t[copy.capacity];
    std::memcpy(this->data, copy.data, copy.position);
    this->position = copy.position;
    return *this;
}


void BinaryQueue::EnqueueBytes(const void* buffer, uint32_t offset, uint32_t count) {
    if (buffer == nullptr)
        throw std::invalid_argument("buffer is null");

    if (this->position + count > this->capacity)
        throw std::length_error("no enough space in binary queue");

    // kopiuj dane na koniec kolejki
    std::memcpy(
            this->data + this->position,
            reinterpret_cast<const uint8_t*>(buffer) + offset,
            count);
    this->position += count;
}

void BinaryQueue::DequeueBytes(uint32_t count) {
    if (count > this->position)
        throw std::length_error("count is to large");

    std::memmove(
            this->data,
            this->data + count,
            this->position - count);
    this->position -= count;
}

void BinaryQueue::Clear(void) {
    this->position = 0;
}