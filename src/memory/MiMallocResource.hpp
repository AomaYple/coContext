#pragma once

#include <memory_resource>

namespace coContext::internal {
    class MiMallocResource final : public std::pmr::memory_resource {
    public:
        constexpr MiMallocResource() noexcept = default;

        constexpr MiMallocResource(const MiMallocResource &) noexcept = default;

        constexpr auto operator=(const MiMallocResource &) noexcept -> MiMallocResource & = default;

        constexpr MiMallocResource(MiMallocResource &&) noexcept = default;

        constexpr auto operator=(MiMallocResource &&) noexcept -> MiMallocResource & = default;

        ~MiMallocResource() override = default;

        [[nodiscard]] auto do_allocate(std::size_t bytes, std::size_t alignment) -> void * override;

        auto do_deallocate(void *pointer, std::size_t bytes, std::size_t alignment) noexcept -> void override;

        [[nodiscard]] auto do_is_equal(const memory_resource &other) const noexcept -> bool override;
    };
}    // namespace coContext::internal
