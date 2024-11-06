#pragma once

#include "Log.hpp"

namespace coContext {
    class Exception final : public std::exception {
    public:
        explicit Exception(Log &&log);

        [[nodiscard]] auto what() const noexcept -> const char * override;

        [[nodiscard]] auto getLog() noexcept -> Log &;

    private:
        std::string message;
        Log log;
    };
}    // namespace coContext
