#pragma once

#include "../memory/memoryResource.hpp"
#include "../ring/RingBuffer.hpp"
#include "coContext/coroutine/Coroutine.hpp"

#include <queue>
#include <source_location>
#include <sys/resource.h>
#include <unordered_map>

namespace coContext::internal {
    class Submission;

    struct Buffer {
        std::pmr::vector<std::byte> buffer{1024, getMemoryResource()};
        std::size_t offset{};
    };

    class Context {
    public:
        Context();

        Context(const Context &) = delete;

        auto operator=(const Context &) -> Context & = delete;

        Context(Context &&) noexcept = default;

        auto operator=(Context &&) noexcept -> Context & = default;

        ~Context() = default;

        auto swap(Context &other) noexcept -> void;

        auto run() -> void;

        auto stop() noexcept -> void;

        auto spawn(Coroutine coroutine) -> void;

        [[nodiscard]] auto getSubmission() const -> Submission;

        [[nodiscard]] auto syncCancel(std::variant<std::uint64_t, std::int32_t> identity, std::int32_t flags,
                                      __kernel_timespec timeSpecification) const -> std::int32_t;

        [[nodiscard]] auto getRingBufferId() const noexcept -> std::int32_t;

        [[nodiscard]] auto getData(std::uint16_t bufferId, std::size_t dataSize) noexcept -> std::span<const std::byte>;

        auto clearBufferOffset(std::uint16_t bufferId) noexcept -> void;

        auto expandBuffer() -> void;

    private:
        [[nodiscard]] static auto
            getFileDescriptorLimit(std::source_location sourceLocation = std::source_location::current()) -> rlim_t;

        auto scheduleUnscheduledCoroutines() -> void;

        auto scheduleCoroutine(Coroutine coroutine) -> void;

        static constexpr std::uint16_t entries{32768};

        std::shared_ptr<Ring> ring;
        RingBuffer ringBuffer;
        std::pmr::vector<Buffer> bufferGroup{getMemoryResource()};
        std::queue<Coroutine, std::pmr::deque<Coroutine>> unscheduledCoroutines{getMemoryResource()};
        std::pmr::unordered_map<std::uint64_t, Coroutine> schedulingCoroutines{getMemoryResource()};
        bool isRunning{};
    };
}    // namespace coContext::internal

template<>
constexpr auto std::swap(coContext::internal::Context &lhs, coContext::internal::Context &rhs) noexcept -> void {
    lhs.swap(rhs);
}
