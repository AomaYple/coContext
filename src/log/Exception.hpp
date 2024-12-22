#pragma once

#include "Log.hpp"

namespace coContext {
    class Exception final : public std::exception {
    public:
        explicit Exception(Log log = Log{});

        auto swap(Exception &other) noexcept -> void;

        [[nodiscard]] auto what() const noexcept -> const char * override;

        [[nodiscard]] auto getLog() noexcept -> Log &;

    private:
        std::pmr::string message;
        Log log;
    };

}    // namespace coContext

template<>
constexpr auto std::swap(coContext::Exception &lhs, coContext::Exception &rhs) noexcept -> void {
    lhs.swap(rhs);
}
