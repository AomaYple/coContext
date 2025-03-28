#include <arpa/inet.h>
#include <coContext/coContext.hpp>

using namespace std::string_view_literals;

[[nodiscard]] auto acceptAction(const std::int32_t socket) -> coContext::Task<> {
    co_await coContext::multipleReceive(
        [socket]([[maybe_unused]] const std::int32_t result,
                 [[maybe_unused]] const std::span<const std::byte> receivedData) -> coContext::Task<> {
            static constexpr auto response{
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 0\r\n"
                "\r\n"sv};

            co_await (coContext::send(socket, std::as_bytes(std::span{response}), 0) | coContext::direct());
        },
        socket, 0, coContext::direct());

    co_await coContext::closeDirect(socket);
}

[[nodiscard]] auto server() -> coContext::Task<> {
    const std::int32_t socket{co_await coContext::directSocket(AF_INET, SOCK_STREAM, 0)};

    std::int32_t option{1};
    co_await (coContext::setSocketOption(socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                         std::as_writable_bytes(std::span{std::addressof(option), 1})) |
              coContext::direct());

    sockaddr_in address{};
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);

    co_await (coContext::bind(socket, reinterpret_cast<sockaddr *>(std::addressof(address)), sizeof(address)) |
              coContext::direct());

    co_await (coContext::listen(socket, SOMAXCONN) | coContext::direct());

    co_await multipleAcceptDirect(acceptAction, socket, nullptr, nullptr, 0, coContext::direct());

    co_await coContext::closeDirect(socket);
}

constexpr auto execute() {
    spawn(server);

    coContext::run();
}

[[nodiscard]] auto main() -> int {
    coContext::logger::stop();
    coContext::logger::disableWrite();

    std::vector<std::jthread> workers;
    for (std::uint8_t i{}; i != std::thread::hardware_concurrency() - 1; ++i) workers.emplace_back(execute);

    execute();
}
