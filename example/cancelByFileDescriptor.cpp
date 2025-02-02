#include <arpa/inet.h>
#include <coContext/coContext.hpp>
#include <print>

[[nodiscard]] auto cancelFunction(const std::int32_t socket) -> coContext::Task<> {
    std::println("cancel result: {}", co_await coContext::cancel(socket));
}

[[nodiscard]] auto function() -> coContext::Task<> {
    const std::int32_t socket{co_await coContext::socket(AF_INET, SOCK_STREAM, 0)};

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(12345);
    address.sin_addr.s_addr = INADDR_ANY;

    co_await coContext::bind(socket, reinterpret_cast<sockaddr &>(address), sizeof(address));

    co_await coContext::listen(socket, SOMAXCONN);

    spawn(cancelFunction, socket);

    std::println("accept result: {}", co_await coContext::accept(socket, nullptr, nullptr));
}

auto main() -> int {
    spawn(function);

    coContext::run();
}
