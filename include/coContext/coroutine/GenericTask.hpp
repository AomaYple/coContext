#pragma once

#include "Coroutine.hpp"

#include <cstdint>

namespace coContext {
    class GenericTask {
    public:
        explicit GenericTask(Coroutine &&coroutine) noexcept;

        GenericTask(const GenericTask &) = delete;

        auto operator=(const GenericTask &) -> GenericTask & = delete;

        GenericTask(GenericTask &&) noexcept = default;

        auto operator=(GenericTask &&) noexcept -> GenericTask & = default;

        ~GenericTask() = default;

        auto swap(GenericTask &other) noexcept -> void;

        [[nodiscard]] auto getCoroutine() const noexcept -> const Coroutine &;

        auto setResult(std::int32_t result) noexcept -> void;

        [[nodiscard]] auto getResult() const noexcept -> std::int32_t;

    private:
        Coroutine coroutine;
        std::int32_t result{};
    };
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::GenericTask &lhs, coContext::GenericTask &rhs) noexcept -> void {
    lhs.swap(rhs);
}
