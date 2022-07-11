#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>
#include <memory>
#include <iostream>

template <typename T>
class RawMemory {
public:
    RawMemory() = default;
    RawMemory(const RawMemory&) = delete;
    RawMemory& operator=(const RawMemory& rhs) = delete;

    RawMemory(RawMemory&& other) noexcept {
        Swap(other);
    }
    RawMemory& operator=(RawMemory&& rhs) noexcept {
        Swap(rhs);
        return *this;
    }

    explicit RawMemory(size_t capacity)
        : buffer_(Allocate(capacity))
        , capacity_(capacity) {
    }

    ~RawMemory() {
        Deallocate(buffer_);
    }

    T* operator+(size_t offset) noexcept {
        // Разрешается получать адрес ячейки памяти, следующей за последним элементом массива
        assert(offset <= capacity_);
        return buffer_ + offset;
    }

    const T* operator+(size_t offset) const noexcept {
        return const_cast<RawMemory&>(*this) + offset;
    }

    const T& operator[](size_t index) const noexcept {
        return const_cast<RawMemory&>(*this)[index];
    }

    T& operator[](size_t index) noexcept {
        assert(index < capacity_);
        return buffer_[index];
    }

    void Swap(RawMemory& other) noexcept {
        std::swap(buffer_, other.buffer_);
        std::swap(capacity_, other.capacity_);
    }

    const T* GetAddress() const noexcept {
        return buffer_;
    }

    T* GetAddress() noexcept {
        return buffer_;
    }

    size_t Capacity() const {
        return capacity_;
    }

private:
    // Выделяет сырую память под n элементов и возвращает указатель на неё
    static T* Allocate(size_t n) {
        return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
    }

    // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
    static void Deallocate(T* buf) noexcept {
        operator delete(buf);
    }

    T* buffer_ = nullptr;
    size_t capacity_ = 0;
};

template <typename T>
class Vector {
public:
    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() noexcept
    {
        return data_.GetAddress();
    }
    iterator end() noexcept
    {
        return data_.GetAddress()+size_;
    }
    const_iterator begin() const noexcept
    {
        return data_.GetAddress();
    }
    const_iterator end() const noexcept
    {
        return data_.GetAddress()+size_;
    }
    const_iterator cbegin() const noexcept
    {
        return data_.GetAddress();
    }
    const_iterator cend() const noexcept
    {
        return data_.GetAddress()+size_;
    }

    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args)
    {
        //new (new_data + size_) T(std::forward<Args>(args)...);
        int shift = pos - data_.GetAddress();
        iterator p = data_.GetAddress() + shift;

        if (size_<data_.Capacity())
        {
            if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
                std::move_backward(p, data_.GetAddress()+size_-1, data_.GetAddress()+size_);
            }
            else
            {
                std::copy_backward(p, data_.GetAddress()+size_-1, data_.GetAddress()+size_);
            }


            new(p) T(std::forward<Args>(args)...);
        }
        else
        {
            RawMemory<T> new_data(size_ ? size_*2:1);
            new(new_data.GetAddress() + shift) T(std::forward<Args>(args)...);

            if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
                std::uninitialized_move_n(data_.GetAddress(), shift, new_data.GetAddress());
                std::uninitialized_move_n(data_.GetAddress()+shift, size_-shift, new_data.GetAddress()+shift+1);
                }
            else {
                std::uninitialized_copy_n(data_.GetAddress(), shift, new_data.GetAddress());
                std::uninitialized_copy_n(data_.GetAddress()+shift, size_-shift, new_data.GetAddress()+shift +1);
                }
            std::destroy_n(data_.GetAddress(), size_);
            data_.Swap(new_data);
        }
        size_++;
        return data_.GetAddress() + shift;

    }

    iterator Insert(const_iterator pos, const T& value)
    {
        return Emplace(pos, value);
        //int shift = pos - data_.GetAddress();
        //iterator p = data_.GetAddress() + shift;

        //if (size_<data_.Capacity())
        //{
        //    std::move_backward(p, end(), p+1);
        //    new(p) T(value);
        //}
        //else
        //{
        //    RawMemory<T> new_data(size_?size_*2:1);
        //    new(new_data.GetAddress() + shift) T(value);

        //    if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
        //        std::uninitialized_move_n(data_.GetAddress(), shift, new_data.GetAddress());
        //        std::uninitialized_move_n(data_.GetAddress()+shift, size_-shift, new_data.GetAddress()+shift+1);
        //        }
        //    else {
        //        std::uninitialized_copy_n(data_.GetAddress(), shift, new_data.GetAddress());
        //        std::uninitialized_copy_n(data_.GetAddress()+shift, size_-shift, new_data.GetAddress()+shift +1);
        //        }
        //    std::destroy_n(data_.GetAddress(), size_);
        //    data_.Swap(new_data);
        //}
        //size_++;
        //return data_.GetAddress() + shift;
    }

    iterator Insert(const_iterator pos, T&& value)
    {
        return Emplace(pos, value);
        //int shift = pos - data_.GetAddress();
        //iterator p = data_.GetAddress() + shift;

        //if (size_<data_.Capacity())
        //{
        //    //std::move_backward(p, end(), p+1);
        //    if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
        //        std::move_backward(p, data_.GetAddress()+size_-1, data_.GetAddress()+size_);
        //    }
        //    else
        //    {
        //        std::copy_backward(p, data_.GetAddress()+size_-1, data_.GetAddress()+size_);
        //    }
        //    new(p) T(std::move(value));
        //}
        //else
        //{
        //    RawMemory<T> new_data(size_?size_*2:1);
        //    new(new_data.GetAddress() + shift) T(std::move(value));

        //    if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
        //        std::uninitialized_move_n(data_.GetAddress(), shift, new_data.GetAddress());
        //        std::uninitialized_move_n(data_.GetAddress()+shift, size_-shift, new_data.GetAddress()+shift+1);
        //        }
        //    else {
        //        std::uninitialized_copy_n(data_.GetAddress(), shift, new_data.GetAddress());
        //        std::uninitialized_copy_n(data_.GetAddress()+shift, size_-shift, new_data.GetAddress()+shift +1);
        //        }
        //    std::destroy_n(data_.GetAddress(), size_);
        //    data_.Swap(new_data);
        //}
        //size_++;
        //return data_.GetAddress() + shift;
    }

    iterator Erase(const_iterator pos)
    {
    }

    Vector() = default;

    explicit Vector(size_t size)
        : data_(size)
        , size_(size)  //
    {
        std::uninitialized_value_construct_n(data_.GetAddress(), size);
    }

    Vector(const Vector& other)
            : data_(other.size_)
            , size_(other.size_)
    {
        std::uninitialized_copy_n(other.data_.GetAddress(), other.size_, data_.GetAddress());
    }

    Vector(Vector&& other) noexcept
    {
        Swap(other);
    }

    ~Vector() {
        std::destroy_n(data_.GetAddress(), size_);
    }

    Vector& operator=(Vector&& rhs) noexcept{
        //Vector<T> tmp(rhs);
        Swap(rhs);
        return *this;
    }

    Vector& operator=(const Vector& rhs) {
            if (this != &rhs) {
                if (rhs.size_ > data_.Capacity()) {
                    Vector rhs_copy(rhs);
                    Swap(rhs_copy);
                }
                else {

                    if (rhs.size_ < size_)
                    {
                        //for (size_t i = 0; i < rhs.size_; i++)
                        //    *(data_.GetAddress()+i) = *(rhs.data_.GetAddress()+i);
                        std::copy(rhs.data_.GetAddress(), rhs.data_.GetAddress() + rhs.size_, data_.GetAddress());
                        std::destroy_n(data_.GetAddress()+rhs.size_, size_-rhs.size_);
                    }
                    else
                    {
                        //for (size_t i = 0; i < size_; i++)
                        //    *(data_.GetAddress()+i) = *(rhs.data_.GetAddress()+i);
                        //
                        std::copy(rhs.data_.GetAddress(), rhs.data_.GetAddress() + size_, data_.GetAddress());
                        std::uninitialized_copy_n(rhs.data_.GetAddress()+size_, rhs.size_-size_, data_.GetAddress()+size_);
                    }
                    size_ = rhs.size_;
                }
            }
            return *this;
        }

    void Swap(Vector& other) noexcept
    {
        std::swap(this->data_, other.data_);
        std::swap(this->size_, other.size_);
    }

    size_t Size() const noexcept {
        return size_;
    }

    size_t Capacity() const noexcept {
        return data_.Capacity();
    }

    const T& operator[](size_t index) const noexcept {
        return const_cast<Vector&>(*this)[index];
    }

    T& operator[](size_t index) noexcept {
        assert(index < size_);
        return data_[index];
    }

    void Resize(size_t new_size)
    {
        if(new_size < size_)
        {
            std::destroy_n(data_.GetAddress()+new_size, size_ - new_size);
            size_ = new_size;
        }
        else
        {
            Reserve(new_size);
            std::uninitialized_value_construct_n(data_.GetAddress()+size_, new_size - size_);
        }
        size_ = new_size;
    }

    void PushBack(const T& value)
    {
        if (size_ < data_.Capacity())
            new (data_ + size_) T((value));
        else
        {
            RawMemory<T> new_data(size_?size_*2:1);
            new (new_data + size_) T((value));
            SwapData(new_data);
        }
        size_++;
    }

    void PushBack(T&& value)
    {
        if (size_ < data_.Capacity())
            new (data_ + size_) T(std::move(value));
        else
        {
            RawMemory<T> new_data(size_?size_*2:1);
            new (new_data + size_) T(std::move(value));
            SwapData(new_data);
        }
        size_++;
    }

    template <typename... Args>
    T& EmplaceBack(Args&&... args)
    {
        if (size_ < data_.Capacity())
            new (data_ + size_) T(std::forward<Args>(args)...);
        else
        {
            RawMemory<T> new_data(size_?size_*2:1);
            new (new_data + size_) T(std::forward<Args>(args)...);
            SwapData(new_data);
        }
        size_++;
        return *(data_.GetAddress()+size_ -1);
    }

    void PopBack()  noexcept
    {
        std::destroy(data_.GetAddress()+size_-1, data_.GetAddress()+size_);
        size_--;
    }

    void Reserve(size_t new_capacity) {

        if (new_capacity <= data_.Capacity()) {
            return;
        }
        RawMemory<T> new_data(new_capacity);
        SwapData(new_data);
    }

private:
    void SwapData(RawMemory<T> &new_data)
    {
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
            std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
            }
        else {
            std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
            }
        std::destroy_n(data_.GetAddress(), size_);
        data_.Swap(new_data);
    }
    RawMemory<T> data_;
    size_t size_ = 0;
};
