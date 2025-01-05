#pragma once

#include "../ring/Ring.hpp"
#include "../ring/RingBuffer.hpp"
#include "coContext/coroutine/Coroutine.hpp"
#include "coContext/memory/memoryResource.hpp"

#include <queue>
#include <sys/resource.h>

namespace coContext::internal {
    class Submission;

    class Context {
    public:
        Context();

        Context(const Context &) = delete;

        auto operator=(const Context &) -> Context & = delete;

        Context(Context &&) noexcept = default;

        auto operator=(Context &&) noexcept -> Context & = default;

        ~Context() = default;

        auto swap(Context &other) noexcept -> void;

        [[nodiscard]] auto getRingBuffer() const noexcept -> std::shared_ptr<RingBuffer>;

        auto run() -> void;

        auto stop() noexcept -> void;

        auto spawn(Coroutine coroutine) -> void;

        [[nodiscard]] auto getSubmission() const -> Submission;

        [[nodiscard]] auto syncCancel(std::variant<std::uint64_t, std::int32_t> id, std::int32_t flags,
                                      __kernel_timespec timeSpecification) const -> std::int32_t;

    private:
        [[nodiscard]] static auto
            getFileDescriptorLimit(std::source_location sourceLocation = std::source_location::current()) -> rlim_t;

        auto scheduleUnscheduledCoroutines() -> void;

        auto scheduleCoroutine(Coroutine coroutine) -> void;

        static constexpr std::uint16_t entries{32768};

        std::shared_ptr<Ring> ring{[] {
            io_uring_params parameters{};
            parameters.flags = IORING_SETUP_SUBMIT_ALL | IORING_SETUP_COOP_TASKRUN | IORING_SETUP_TASKRUN_FLAG |
                               IORING_SETUP_SINGLE_ISSUER | IORING_SETUP_DEFER_TASKRUN;

            return std::allocate_shared<Ring>(std::pmr::polymorphic_allocator{getMemoryResource()}, entries,
                                              parameters);
        }()};
        std::shared_ptr<RingBuffer> ringBuffer{std::allocate_shared<RingBuffer>(
            std::pmr::polymorphic_allocator{getMemoryResource()}, this->ring, entries, 0, IOU_PBUF_RING_INC)};
        std::queue<Coroutine, std::pmr::deque<Coroutine>> unscheduledCoroutines{getMemoryResource()};
        std::pmr::unordered_map<std::uint64_t, Coroutine> schedulingCoroutines{getMemoryResource()};
        bool isRunning{};
    };
}    // namespace coContext::internal

template<>
constexpr auto std::swap(coContext::internal::Context &lhs, coContext::internal::Context &rhs) noexcept -> void {
    lhs.swap(rhs);
}
