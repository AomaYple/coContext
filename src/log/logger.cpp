#include "coContext/log/logger.hpp"

#include "LoggerImpl.hpp"

namespace {
    coContext::internal::LoggerImpl loggerImpl;
}    // namespace

auto coContext::logger::run() -> void { loggerImpl.run(); }

auto coContext::logger::stop() noexcept -> void { loggerImpl.stop(); }

auto coContext::logger::setOutputStream(std::ostream *const outputStream) noexcept -> void {
    loggerImpl.setOutputStream(outputStream);
}

auto coContext::logger::enableWrite() noexcept -> void { loggerImpl.enableWrite(); }

auto coContext::logger::disableWrite() noexcept -> void { loggerImpl.disableWrite(); }

auto coContext::logger::getLevel() noexcept -> Log::Level { return loggerImpl.getLevel(); }

auto coContext::logger::setLevel(const Log::Level level) noexcept -> void { loggerImpl.setLevel(level); }

auto coContext::logger::write(Log log) -> void { loggerImpl.write(std::move(log)); }

auto coContext::logger::flush() -> void { loggerImpl.flush(); }
