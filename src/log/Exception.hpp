#pragma once

#include <string>

namespace coContext {
    class Exception final : public std::exception {
    public:
        explicit Exception(std::string &&message = {});

        auto swap(Exception &other) noexcept -> void;

        [[nodiscard]] auto what() const noexcept -> const char * override;

    private:
        std::string message;
    };

    [[nodiscard]] auto operator==(const Exception &lhs, const Exception &rhs) noexcept -> bool;
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::Exception &lhs, coContext::Exception &rhs) noexcept -> void {
    lhs.swap(rhs);
}
