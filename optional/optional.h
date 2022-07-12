#include <stdexcept>
#include <utility>

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional {
public:
    Optional() = default;

    Optional(const T& value)
    {
        obj_ = new (data_) T(value);
        is_initialized_ = true;
    }
    Optional(T&& value)
    {
        obj_ = new (data_) T(std::move(value));
        is_initialized_ = true;

    }

    Optional(const Optional& other)
    {
        is_initialized_ = false;
        if (other.HasValue())
        {
            obj_ = new (data_) T(other.Value());
            is_initialized_ = true;
        }
    }

    Optional(Optional&& other)
    {
        is_initialized_ = false;
        if (other.HasValue())
        {
            obj_ = new (data_) T(std::move(other.Value()));
            is_initialized_ = true;
        }
    }

    Optional& operator=(const T& value)
    {
        if (HasValue())
        {
            *obj_ = value;
        }
        else
        {
            obj_ = new (data_) T(value);
        }
        is_initialized_ = true;
        return *this;

    }

    Optional& operator=(T&& rhs)
    {
        if (HasValue())
        {
            *obj_ = std::move(rhs);
        }
        else
        {
            obj_ = new (data_) T(std::move(rhs));
        }
        is_initialized_ = true;
        return *this;
    }

    Optional& operator=(const Optional& rhs)
    {
        if (rhs.HasValue())
        {
            if (HasValue())
            {
                *obj_ = rhs.Value();
            }
            else
            {
                obj_ = new (data_) T(rhs.Value());
            }
            is_initialized_ = true;
        }
        else if (HasValue())
            Reset();

        return *this;
    }

    Optional& operator=(Optional&& rhs)
    {
        if (rhs.HasValue())
        {
            if (HasValue())
            {
                *obj_ = std::move(rhs.Value());
            }
            else
            {
                obj_ = new (data_) T(std::move(rhs.Value()));
            }
            is_initialized_ = true;
        }
        else if (HasValue())
            Reset();

        return *this;

    }

    ~Optional()
    {
        Reset();
    }

    template <typename... U>
    void Emplace(U&&... arg)
    {
        if (HasValue())
        {
            Reset();
        }
        obj_ = new (data_) T(std::forward<U>(arg)...);
        is_initialized_ = true;
    }

    bool HasValue() const
    {
        return is_initialized_;
    }

    // Операторы * и -> не должны делать никаких проверок на пустоту Optional.
    // Эти проверки остаются на совести программиста
    T& operator*() &
    {
        return *obj_;
    }
    const T& operator*() const &
    {
        return *obj_;
    }

    T* operator->()
    {
        return obj_;
    }

    const T* operator->() const
    {
        return obj_;
    }

    T&& operator*() && {
         return std::move(*obj_);
    }

    T&& Value() &&
    {
        if (!HasValue())
            throw BadOptionalAccess{};
        return std::move(*obj_);
    }

    T& Value() &
    {
        if (!HasValue())
            throw BadOptionalAccess{};
        return *obj_;
    }
    const T& Value() const &
    {
        if (!HasValue())
            throw BadOptionalAccess{};
        return *obj_;
    }

    void Reset()
    {
        if (is_initialized_)
        {
            is_initialized_ = false;
            obj_->~T();
            obj_ = nullptr;
        }

    }

private:
    // alignas нужен для правильного выравнивания блока памяти
    alignas(T) char data_[sizeof(T)];
    bool is_initialized_ = false;
    T *obj_ = nullptr;
};
