#include "Logger.hpp"

#include <utility>

coContext::internal::Logger::Logger(Logger &&other) noexcept :
    level{other.level}, logs{std::move(other.logs)}, fileDescriptor{std::exchange(other.fileDescriptor, -1)} {}

auto coContext::internal::Logger::operator=(Logger &&other) noexcept -> Logger & {
    if (this == std::addressof(other)) return *this;

    this->~Logger();

    this->level = other.level;
    this->logs = std::move(other.logs);
    this->fileDescriptor = std::exchange(other.fileDescriptor, -1);

    return *this;
}

coContext::internal::Logger::~Logger() {
    if (this->fileDescriptor != -1) close(this->fileDescriptor);
}

auto coContext::internal::Logger::swap(Logger &other) noexcept -> void {
    std::swap(this->level, other.level);
    std::swap(this->logs, other.logs);
    std::swap(this->fileDescriptor, other.fileDescriptor);
}

auto coContext::internal::Logger::setLevel(const Log::Level level) noexcept -> void { this->level = level; }

auto coContext::internal::Logger::getFileDescriptor() const noexcept -> std::int32_t { return this->fileDescriptor; }

auto coContext::internal::Logger::read() -> std::pmr::vector<std::byte> {
    std::pmr::vector<std::byte> bytes{getMemoryResource()};
    for (const auto &log : this->logs) {
        const std::pmr::vector logBytes{log.toBytes()};
        bytes.insert(std::cend(bytes), std::cbegin(logBytes), std::cend(logBytes));
    }

    this->logs.clear();

    return bytes;
}

auto coContext::internal::Logger::write(Log log, const std::source_location sourceLocation) -> void {
    if (log.getLevel() < this->level) return;

    this->logs.emplace_back(std::move(log));

    if (const std::int32_t result{eventfd_write(this->fileDescriptor, 1)}; result == -1) {
        throw Exception{
            Log{Log::Level::fatal,
                std::pmr::string{std::error_code{errno, std::generic_category()}.message(), getMemoryResource()},
                sourceLocation}
        };
    }
}
