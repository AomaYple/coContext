#pragma once

#include <exception>
#include <string>

namespace coContext {
    class Exception final : public std::exception {
    public:
        explicit Exception(const char *message);

        explicit Exception(std::string_view message);

        explicit Exception(std::string &&message) noexcept;

        [[nodiscard]] auto what() const noexcept -> const char * override;

    private:
        std::string message;
    };
}    // namespace coContext
