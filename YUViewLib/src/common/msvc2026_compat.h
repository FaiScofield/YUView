#ifndef Msvc2022Compat_H
#define Msvc2022Compat_H
#pragma once
#include <iterator>

// Compatibility shim for VS 2022 which removed stdext::checked_array_iterator
// and stdext::make_checked_array_iterator. These are no-ops that return the
// pointer unchanged.
namespace stdext {
    template <typename _Ptr>
    _Ptr make_checked_array_iterator(_Ptr _Ptr_, size_t) noexcept {
        return _Ptr_;
    }
}
#endif /* Msvc2022Compat_H */