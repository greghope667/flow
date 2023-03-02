#pragma once

#include "flow_assert.h"
#include <range/v3/view/iota.hpp>
#include <range/v3/view/filter.hpp>

namespace flow {

template <typename T>
struct vector_pool
{
    int acquire_object() {
        throw_assert(spaces_.size() == vec_.size(), "pool vectors must be same size");

        int sz = spaces_.size();
        for (int i=0; i<sz; i++) {
            if (spaces_.at(i)) {
                return i;
            }
        }

        // No space
        int new_index = vec_.size();
        vec_.emplace_back();
        spaces_.push_back(false);
        return new_index;
    }

    void release_object(int index) {
        throw_assert(spaces_.at(index) == false, "double-release detected");
        vec_.at(index) = T{};
        spaces_.at(index) = true;
    }

    T& at(int index) {
        throw_assert(spaces_.at(index) == false, "attempted access of invalid object");
        return vec_.at(index);
    }

    auto valid_indexes() const {
        auto alive = [this](int i){ return not spaces_.at(i); };

        namespace v = ranges::views;
        return v::iota(0, int(spaces_.size())) | v::filter(alive);
    }

    auto& vector() {
        return vec_;
    }

private:
    std::vector<T> vec_;
    std::vector<bool> spaces_;
};

}
