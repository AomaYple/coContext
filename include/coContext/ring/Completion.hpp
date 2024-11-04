#pragma once

#include "Outcome.hpp"

namespace coContext {
    struct Completion {
        Outcome outcome;
        unsigned long userData;
    };
}    // namespace coContext
