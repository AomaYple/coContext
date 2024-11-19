#include "coContext/coContext.hpp"

#include "coContext/coroutine/Context.hpp"

static thread_local coContext::Context context;

auto coContext::run() -> void { context.run(); }

auto coContext::spawn(Task &&task) -> void { context.submit(std::move(task)); }

auto coContext::close(const int fileDescriptor) -> AsyncWaiter { return context.close(fileDescriptor); }
