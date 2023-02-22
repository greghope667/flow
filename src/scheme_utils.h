#pragma once

/* RAII holder for s7 scheme types */

#include <memory>

// Forward declarations for s7 scheme stuff
struct s7_scheme;
struct s7_cell;
using s7_pointer = s7_cell*;

namespace flow {

// Scheme instance is big global variable for now, until I have
// a better idea for what I want to do.
extern s7_scheme* s7;
void scheme_init();
void scheme_free();
void scheme_abort_execution();


// Some basic utilites
const char* pretty_print(s7_pointer obj);
bool scheme_add_resource_path(const char* path);


// RAII container - protects value from gc
class scheme_value
{
    using sv = scheme_value;
public:
    scheme_value(s7_pointer) noexcept;
    scheme_value() noexcept : ptr_(nullptr), loc_(0) {};

    s7_pointer get() noexcept { return ptr_; };
    const char* pretty_print() const noexcept;

    scheme_value(sv&& other) noexcept : scheme_value() { swap(*this, other); };
    scheme_value(const sv& other) noexcept : scheme_value(other.ptr_) {};
    scheme_value& operator=(sv&& other) noexcept;
    scheme_value& operator=(const sv& other) noexcept;
    ~scheme_value() noexcept;

    operator bool() noexcept {
        return ptr_ != nullptr;
    }

    friend void swap(sv& lhs, sv& rhs) noexcept {
        std::swap(lhs.ptr_, rhs.ptr_);
        std::swap(lhs.loc_, rhs.loc_);
    }

private:
    s7_pointer ptr_;
    long loc_;
};


inline scheme_value& scheme_value::operator=(scheme_value&& other) noexcept
{
    scheme_value tmp{std::move(other)};
    swap(*this, tmp);
    return *this;
}

inline scheme_value& scheme_value::operator=(const scheme_value& other) noexcept
{
    scheme_value tmp{other};
    swap(*this, tmp);
    return *this;
}

}