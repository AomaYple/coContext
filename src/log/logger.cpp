#include "coContext/log/logger.hpp"

#include <iostream>
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

    std::atomic<Node *> list;
    std::atomic level{Log::Level::info};
    std::atomic_flag notifyVariable, isDisable;
    std::ostream *outStream{std::addressof(std::cout)};

    constexpr auto output() {
        Node *node{list.exchange(nullptr, std::memory_order::relaxed)};

        std::pmr::vector<Log> logs{getUnSyncMemoryResource()};
        while (node != nullptr) {
            logs.emplace_back(std::move(node->log));

            Node *const next{node->next};
            delete node;
            node = next;
        }

        for (const auto &log : logs | std::views::reverse) std::print(*outStream, "{}"sv, log.toString());
    }

    constexpr auto work(const std::stop_token token) {
        output();

        while (!token.stop_requested()) {
            notifyVariable.wait(false, std::memory_order::relaxed);
            notifyVariable.clear(std::memory_order::relaxed);

            output();
        }
    }

    std::jthread worker{work};

    constexpr auto notify() noexcept {
        notifyVariable.test_and_set(std::memory_order::relaxed);
        notifyVariable.notify_one();
    }
}    // namespace coContext::internal

auto coContext::startLogging() -> void {
    if (!internal::worker.joinable()) internal::worker = std::jthread{internal::work};
}

auto coContext::stopLogging() noexcept -> void {
    internal::worker.request_stop();

    internal::notify();
}

auto coContext::setLevel(const Log::Level level) noexcept -> void {
    internal::level.store(level, std::memory_order::relaxed);
}

auto coContext::enableWriteLog() noexcept -> void { internal::isDisable.clear(std::memory_order::relaxed); }

auto coContext::disableWriteLog() noexcept -> void { internal::isDisable.test_and_set(std::memory_order::relaxed); }

auto coContext::writeLog(Log log) -> void {
    if (internal::isDisable.test(std::memory_order::relaxed) ||
        log.getLevel() < internal::level.load(std::memory_order::relaxed))
        return;

    auto *const node{
        new internal::Node{std::move(log), internal::list.load(std::memory_order::relaxed)}
    };
    while (!internal::list.compare_exchange_weak(node->next, node, std::memory_order::release,
                                                 std::memory_order::relaxed));

    internal::notify();
}

auto coContext::setOutputStream(std::ostream &outStream) noexcept -> void {
    internal::outStream = std::addressof(outStream);
}
