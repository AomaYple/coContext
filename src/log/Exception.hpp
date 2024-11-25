#pragma once

#include <string>

namespace coContext {
    class Exception final : public std::exception {
    public:
        explicit Exception(std::string &&message = {}) noexcept;

        auto swap(Exception &other) noexcept -> void;

        [[nodiscard]] auto what() const noexcept -> const char * override;

        [[nodiscard]] auto getMessage() noexcept -> std::string &;

    private:
        std::string message;
    };
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::Exception &lhs, coContext::Exception &rhs) noexcept -> void {
    lhs.swap(rhs);
}
