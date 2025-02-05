#include "coContext/log/logger.hpp"

#include "LoggerImpl.hpp"

namespace {
    [[nodiscard]] auto getLogger() -> coContext::internal::LoggerImpl & {
        static coContext::internal::LoggerImpl logger;

        return logger;
    }
}    // namespace

auto coContext::logger::run() -> void { getLogger().run(); }

auto coContext::logger::stop() -> void { getLogger().stop(); }

auto coContext::logger::setOutputStream(std::ostream *const outputStream) -> void {
    getLogger().setOutputStream(outputStream);
}

auto coContext::logger::enableWrite() -> void { getLogger().enableWrite(); }

auto coContext::logger::disableWrite() -> void { getLogger().disableWrite(); }

auto coContext::logger::getLevel() -> Log::Level { return getLogger().getLevel(); }

auto coContext::logger::setLevel(const Log::Level level) -> void { getLogger().setLevel(level); }

auto coContext::logger::write(Log log) -> void { getLogger().write(std::move(log)); }

auto coContext::logger::flush() -> void { getLogger().flush(); }
