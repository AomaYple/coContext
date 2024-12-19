#pragma once

#include "../ring/Ring.hpp"
#include "coContext/coroutine/Coroutine.hpp"
#include "coContext/memory/memoryResource.hpp"

#include <queue>
#include <sys/resource.h>
#include <variant>

namespace coContext {
    class SubmissionQueueEntry;

    class Context {
    public:
        Context();

        Context(const Context &) = delete;

        auto operator=(const Context &) -> Context & = delete;

        Context(Context &&) noexcept = default;

        auto operator=(Context &&) noexcept -> Context & = default;

        ~Context();

        auto swap(Context &other) noexcept -> void;

        auto run() -> void;

        auto stop() noexcept -> void;

        auto spawn(Coroutine &&coroutine) -> void;

        [[nodiscard]] auto getSubmissionQueueEntry() -> SubmissionQueueEntry;

        [[nodiscard]] auto syncCancel(std::variant<std::uint64_t, std::int32_t> identity, std::int32_t flags,
                                      __kernel_timespec timeSpecification) -> std::int32_t;

    private:
        [[nodiscard]] static auto
            getFileDescriptorLimit(std::source_location sourceLocation = std::source_location::current()) -> rlim_t;

        auto scheduleUnscheduledCoroutines() -> void;

        static constinit std::mutex mutex;
        static constinit std::int32_t sharedRingFileDescriptor;
        static constinit std::uint32_t cpuCode;

        bool isRunning{};
        Ring ring;
        std::queue<Coroutine, std::pmr::deque<Coroutine>> unscheduledCoroutines{getMemoryResource()};
        std::pmr::unordered_map<std::uint64_t, Coroutine> schedulingCoroutines{getMemoryResource()};
    };
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::Context &lhs, coContext::Context &rhs) noexcept -> void {
    lhs.swap(rhs);
}
