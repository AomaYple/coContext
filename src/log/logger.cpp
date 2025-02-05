#include "coContext/log/logger.hpp"

#include "LoggerImpl.hpp"

namespace {
    [[nodiscard]] auto getLogger() -> coContext::internal::LoggerImpl & {
        static coContext::internal::LoggerImpl logger;

        return logger;
    }
}    // namespace

auto coContext::logger::run() -> void { getLogger().run(); }

auto coContext::logger::stop() noexcept -> void { getLogger().stop(); }

auto coContext::logger::setOutputStream(std::ostream *const outputStream) noexcept -> void {
    getLogger().setOutputStream(outputStream);
}

auto coContext::logger::enableWrite() noexcept -> void { getLogger().enableWrite(); }

auto coContext::logger::disableWrite() noexcept -> void { getLogger().disableWrite(); }

auto coContext::logger::getLevel() noexcept -> Log::Level { return getLogger().getLevel(); }

auto coContext::logger::setLevel(const Log::Level level) noexcept -> void { getLogger().setLevel(level); }

auto coContext::logger::write(Log log) -> void { getLogger().write(std::move(log)); }

auto coContext::logger::flush() -> void { getLogger().flush(); }
