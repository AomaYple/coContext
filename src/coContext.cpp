#include "coContext/coContext.hpp"

#include "coContext/coroutine/Context.hpp"

static thread_local coContext::Context context;

auto coContext::run() -> void { context.run(); }

auto coContext::spawn(Task &&task) -> void { context.submit(std::move(task)); }

auto coContext::close(const std::int32_t fileDescriptor) -> AsyncWaiter { return context.close(fileDescriptor); }

auto coContext::socket(const std::int32_t domain, const std::int32_t type, const std::int32_t protocol) -> AsyncWaiter {
    return context.socket(domain, type, protocol);
}

auto coContext::bind(const std::int32_t socket, sockaddr *const address, const std::uint32_t addressLength)
    -> AsyncWaiter {
    return context.bind(socket, address, addressLength);
}

auto coContext::listen(const std::int32_t socket, const std::int32_t backlog) -> AsyncWaiter {
    return context.listen(socket, backlog);
}

auto coContext::accept(const std::int32_t socket, sockaddr *const address, std::uint32_t *const addressLength)
    -> AsyncWaiter {
    return context.accept(socket, address, addressLength);
}

auto coContext::accept4(const std::int32_t socket, sockaddr *const address, std::uint32_t *const addressLength,
                        const std::int32_t flags) -> AsyncWaiter {
    return context.accept4(socket, address, addressLength, flags);
}

auto coContext::connect(const std::int32_t socket, const sockaddr *const address, const std::uint32_t addressLength)
    -> AsyncWaiter {
    return context.connect(socket, address, addressLength);
}
