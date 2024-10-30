#pragma once

#include <exception>
#include <string>

namespace coRing {
    class Exception final : public std::exception {
    public:
        explicit Exception(std::string &&message) noexcept;

        [[nodiscard]] auto what() const noexcept -> const char * override;

    private:
        std::string message;
    };
}    // namespace coRing
