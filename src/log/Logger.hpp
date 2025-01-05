#pragma once

#include "../memory/memoryResource.hpp"
#include "Log.hpp"

namespace coContext::internal {
    class Logger {
    public:
        constexpr Logger() = default;

    private:
        std::pmr::vector<Log> logs{getMemoryResource()};
    };
}    // namespace coContext::internal
