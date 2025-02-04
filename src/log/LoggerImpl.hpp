#pragma once

#include "coContext/log/Log.hpp"

#include <iostream>

namespace coContext::internal {
    class LoggerImpl {
        struct Node {
            [[nodiscard]] auto operator new(std::size_t bytes) -> void *;

            auto operator delete(void *pointer, std::size_t bytes) noexcept -> void;

            Log log;
            Node *next;
        };

    public:
        LoggerImpl();

        LoggerImpl(const LoggerImpl &) = delete;

        auto operator=(const LoggerImpl &) -> LoggerImpl & = delete;

        LoggerImpl(LoggerImpl &&) noexcept = delete;

        auto operator=(LoggerImpl &&) noexcept -> LoggerImpl & = delete;

        ~LoggerImpl();

        auto run() -> void;

        auto stop() noexcept -> void;

        auto setOutputStream(std::ostream *outputStream) noexcept -> void;

        auto enableWrite() noexcept -> void;

        auto disableWrite() noexcept -> void;

        [[nodiscard]] auto getLevel() const noexcept -> Log::Level;

        auto setLevel(Log::Level level) noexcept -> void;

        auto write(Log log) -> void;

        auto flush() const -> void;

    private:
        [[nodiscard]] static auto reverseList(Node *node) noexcept -> Node *;

        auto notify() noexcept -> void;

        std::jthread worker;
        std::atomic<std::ostream *> outputStream{std::addressof(std::clog)};
        std::atomic<Node *> head;
        std::atomic_flag notifyVariable, writeSwitch;
        std::atomic<Log::Level> level{Log::Level::info};
    };
}    // namespace coContext::internal
