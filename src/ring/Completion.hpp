#pragma once

#include <cstdint>
#include <liburing/io_uring.h>

namespace coContext::internal {
    class Completion {
    public:
        explicit Completion(const io_uring_cqe *handle = nullptr) noexcept;

        [[nodiscard]] auto get() const noexcept -> const io_uring_cqe *;

        [[nodiscard]] auto getUserData() const noexcept -> std::uint64_t;

        [[nodiscard]] auto getResult() const noexcept -> std::int32_t;

        [[nodiscard]] auto getFlags() const noexcept -> std::uint32_t;

    private:
        const io_uring_cqe *handle;
    };

    [[nodiscard]] auto operator==(Completion, Completion) noexcept -> bool;
}    // namespace coContext::internal
