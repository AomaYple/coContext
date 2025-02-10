#include <asio.hpp>
#include <thread>

using namespace std::string_view_literals;

[[nodiscard]] auto client(asio::ip::tcp::socket socket) -> asio::awaitable<void> {
    std::array<std::byte, 1024> buffer;
    while (true) {
        co_await socket.async_receive(asio::buffer(buffer), asio::use_awaitable);

        static constexpr auto response{
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: 0\r\n"
            "\r\n"sv};

        co_await socket.async_send(asio::buffer(response), asio::use_awaitable);
    }
}

[[nodiscard]] auto server() -> asio::awaitable<void> {
    const auto executor{co_await asio::this_coro::executor};

    asio::ip::tcp::acceptor acceptor{
        executor, asio::ip::tcp::endpoint{asio::ip::tcp::v4(), 8080}
    };

    while (true) co_spawn(executor, client(co_await acceptor.async_accept(asio::use_awaitable)), asio::detached);
}

constexpr auto execute() {
    asio::io_context context;

    co_spawn(context, server, asio::detached);

    context.run();
}

[[nodiscard]] auto main() -> int {
    std::vector<std::jthread> workers;
    for (std::uint8_t i{}; i != std::thread::hardware_concurrency() - 1; ++i) workers.emplace_back(execute);

    execute();
}
