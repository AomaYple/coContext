#pragma once

#include "../memory/memoryResource.hpp"
#include "../ring/Ring.hpp"
#include "coContext/coroutine/Coroutine.hpp"

#include <queue>
#include <variant>

namespace coContext::internal {
    class Submission;

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

        auto spawn(Coroutine coroutine) -> void;

        [[nodiscard]] auto getSubmission() -> Submission;

        [[nodiscard]] auto syncCancel(std::variant<std::uint64_t, std::int32_t> identity, std::int32_t flags,
                                      __kernel_timespec timeSpecification) -> std::int32_t;

    private:
        auto scheduleUnscheduledCoroutines() -> void;

        static std::uint32_t fileDescriptorLimit;
        static constinit std::mutex mutex;
        static constinit std::int32_t sharedRingFileDescriptor;
        static std::vector<std::uint32_t> cpuCodes;

        Ring ring;
        std::uint32_t cpuCode;
        bool isRunning{};
        std::queue<Coroutine, std::pmr::deque<Coroutine>> unscheduledCoroutines{getMemoryResource()};
        std::pmr::unordered_map<std::uint64_t, Coroutine> schedulingCoroutines{getMemoryResource()};
    };
}    // namespace coContext::internal

template<>
constexpr auto std::swap(coContext::internal::Context &lhs, coContext::internal::Context &rhs) noexcept -> void {
    lhs.swap(rhs);
}
