#pragma once

#include "Promise.hpp"

namespace coContext {
    class Task {
    public:
        using promise_type = Promise;

        explicit Task(std::coroutine_handle<Promise> handle) noexcept;

        Task(const Task &) = delete;

        auto operator=(const Task &) -> Task & = delete;

        Task(Task &&) noexcept;

        auto operator=(Task &&) noexcept -> Task &;

        ~Task();

        auto swap(Task &other) noexcept -> void;

        [[nodiscard]] auto getHash() const noexcept -> std::size_t;

        auto operator()(std::int32_t result) const -> void;

        [[nodiscard]] auto done() const noexcept -> bool;

    private:
        auto destroy() const -> void;

        std::coroutine_handle<Promise> handle;
    };

    [[nodiscard]] auto operator==(const Promise &lhs, const Promise &rhs) noexcept -> bool;
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::Task &lhs, coContext::Task &rhs) noexcept -> void {
    lhs.swap(rhs);
}
