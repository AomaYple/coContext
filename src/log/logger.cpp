#include "coContext/log/logger.hpp"

#include <print>
#include <ranges>

using namespace std::string_view_literals;

namespace coContext::internal {
    struct Node {
        [[nodiscard]] auto operator new(const std::size_t bytes) -> void * {
            return getSyncMemoryResource()->allocate(bytes);
        }

        auto operator delete(void *const pointer, const std::size_t bytes) noexcept -> void {
            getSyncMemoryResource()->deallocate(pointer, bytes);
        }

        Log log;
        Node *next;
    };

    std::atomic_flag isOn, notifyVariable;
    std::atomic<Node *> list;
    std::atomic level{Log::Level::info};
    std::jthread worker;

    constexpr auto notify() noexcept {
        notifyVariable.test_and_set(std::memory_order::relaxed);
        notifyVariable.notify_one();
    }

    constexpr auto output() {
        Node *node{list.exchange(nullptr, std::memory_order::relaxed)};

        std::pmr::vector<Log> logs{getUnSyncMemoryResource()};
        while (node != nullptr) {
            logs.emplace_back(std::move(node->log));

            Node *const next{node->next};
            delete node;
            node = next;
        }

        for (const auto &log : logs | std::views::reverse) std::print("{}"sv, log.toString());
    }
}    // namespace coContext::internal

auto coContext::onLogging() -> void {
    if (internal::isOn.test(std::memory_order::relaxed)) return;

    internal::isOn.test_and_set(std::memory_order::relaxed);

    startLogging();
}

auto coContext::offLogging() noexcept -> void {
    internal::isOn.clear(std::memory_order::relaxed);

    stopLogging();
}

auto coContext::startLogging() -> void {
    if (internal::worker.joinable()) return;

    internal::worker = std::jthread{[](const std::stop_token token) {
        internal::output();

        while (internal::isOn.test(std::memory_order::relaxed) && !token.stop_requested()) {
            internal::notifyVariable.wait(false, std::memory_order::relaxed);
            internal::notifyVariable.clear(std::memory_order::relaxed);

            internal::output();
        }
    }};
}

auto coContext::stopLogging() noexcept -> void {
    internal::worker.request_stop();

    internal::notify();
}

auto coContext::setLevel(const Log::Level level) noexcept -> void {
    if (internal::isOn.test(std::memory_order::relaxed)) internal::level.store(level, std::memory_order::relaxed);
}

auto coContext::writeLog(Log log) -> void {
    if (internal::isOn.test(std::memory_order::relaxed) ||
        log.getLevel() < internal::level.load(std::memory_order::relaxed))
        return;

    auto *const node{
        new internal::Node{std::move(log), internal::list.load(std::memory_order::relaxed)}
    };
    while (!internal::list.compare_exchange_weak(node->next, node, std::memory_order::release,
                                                 std::memory_order::relaxed));

    internal::notify();
}

