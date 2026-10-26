#include "core/conversation.h"

#include <cstddef>
#include <stdexcept>

//using namespace std;

// Conversation container
Conversation::Conversation() {
    size_ = 0;
    capacity_ = 0;
    data_ = nullptr;
}

// Destructor
Conversation::~Conversation() {
    delete [] data_;
}

// Copy constructor
Conversation::Conversation(const Conversation& other) {
    size_ = other.size_;
    capacity_ = other.capacity_;
    data_ = new Message[other.capacity_];
    if (other.data_) {
        // is not empty
        for (std::size_t i = 0; i < capacity_; i++) {
            data_[i] = other.data_[i];
        }
    }
}

// Copy assignment
Conversation& Conversation::operator=(const Conversation& other) {
    if (this != &other) {
        delete [] data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        data_ = new Message[other.capacity_];
        if (other.data_) {
            for (std::size_t i = 0; i < capacity_; i++) {
                data_[i] = other.data_[i];
            }
        }
    }
    return *this;
}

// Move constructor
Conversation::Conversation(Conversation&& other) noexcept {
    // steal pointer and attributes
    size_ = other.size_;
    capacity_ = other.capacity_;
    data_ = other.data_;
    // zero out source
    other.size_ = 0;
    other.capacity_ = 0;
    other.data_ = nullptr;
}

// Move assignment operator
Conversation& Conversation::operator=(Conversation&& other) noexcept {
    if (this != &other) {
        // deallocate current array
        delete [] data_;
        // steal pointer and attributes
        size_ = other.size_;
        capacity_ = other.capacity_;
        data_ = other.data_;
        // zero out source
        other.size_ = 0;
        other.capacity_ = 0;
        other.data_ = nullptr;
    }
    return *this;
}

// Appends a message, growing array if necessary
void Conversation::append(Message m) {
    std::size_t newCapacity = capacity_ * 2;
    // if no array (just initialized), create it
    if (capacity_ == 0) {
        data_ = new Message[1];
        capacity_ = 1;
    }
    // if array is full, make a new array twice the size and copy everything over
    if (size_ == capacity_) {
        Message* data_tmp = data_; // temporarily store old data_ pointer
        data_ = new Message[newCapacity]; // allocate array double size
        capacity_ = newCapacity; // update capacity
        for (std::size_t i = 0; i < size_; i++) {
            data_[i] = data_tmp[i]; // copy array contents
        }
        delete [] data_tmp; // delete old array
    }
    data_[size_] = m; // append message
    size_++; // update size

}

// get number of messages stored
std::size_t Conversation::size() const noexcept {
    return size_;
}

// get a specific Message (check bounds) return null if out of range
const Message& Conversation::at(std::size_t i) const {
    if (i >= size_) {
        throw std::out_of_range("Value entered is out of bounds"); // throw out of bounds exception
    }
    return data_[i];
}

// oldest message
const Message* Conversation::begin() const noexcept{
    if (size_ == 0) {
        return nullptr;
    }
    return data_;
}

// newest message
const Message* Conversation::end() const noexcept {
    if (size_ == 0) {
        return nullptr;
    }
    return data_ + size_;
}
