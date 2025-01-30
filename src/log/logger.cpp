#include "coContext/log/logger.hpp"

#include <iostream>
#include <ranges>

using namespace std::string_view_literals;

namespace {
    struct Node {
        [[nodiscard]] constexpr auto operator new(const std::size_t bytes) -> void * {
            return coContext::internal::getSyncMemoryResource()->allocate(bytes);
        }

        constexpr auto operator delete(void *const pointer, const std::size_t bytes) noexcept -> void {
            coContext::internal::getSyncMemoryResource()->deallocate(pointer, bytes);
        }

        coContext::Log log;
        Node *next;
    };

    std::ostream *outputStream{std::addressof(std::clog)};
    std::atomic<Node *> list;
    std::atomic_flag notifyVariable, isDisableWrite;
    std::atomic level{coContext::Log::Level::info};

    constexpr auto output() {
        Node *node{list.exchange(nullptr, std::memory_order::relaxed)};

        std::pmr::vector<coContext::Log> logs{coContext::internal::getUnSyncMemoryResource()};
        while (node != nullptr) {
            logs.emplace_back(std::move(node->log));

            Node *const next{node->next};
            delete node;
            node = next;
        }

        for (const auto &log : logs | std::views::reverse) std::println(*outputStream, "{}"sv, log);
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
}    // namespace

auto coContext::logger::run() -> void {
    if (!worker.joinable()) worker = std::jthread{work};
}

auto coContext::logger::stop() noexcept -> void {
    worker.request_stop();

    notify();
}

auto coContext::logger::setOutputStream(std::ostream *const newOutputStream) noexcept -> void {
    outputStream = newOutputStream;
}

auto coContext::logger::enableWrite() noexcept -> void { isDisableWrite.clear(std::memory_order::relaxed); }

auto coContext::logger::disableWrite() noexcept -> void { isDisableWrite.test_and_set(std::memory_order::relaxed); }

auto coContext::logger::getLevel() noexcept -> Log::Level { return level.load(std::memory_order::relaxed); }

auto coContext::logger::setLevel(const Log::Level newLevel) noexcept -> void {
    level.store(newLevel, std::memory_order::relaxed);
}

auto coContext::logger::write(Log log) -> void {
    if (isDisableWrite.test(std::memory_order::relaxed) || log.getLevel() < level.load(std::memory_order::relaxed))
        return;

    auto *const node{
        new Node{std::move(log), list.load(std::memory_order::relaxed)}
    };
    while (!list.compare_exchange_weak(node->next, node, std::memory_order::release, std::memory_order::relaxed));

    notify();
}
