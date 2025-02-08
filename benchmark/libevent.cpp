#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <thread>
#include <vector>

using namespace std::string_view_literals;

constexpr auto receiveAction(bufferevent *const bufferEvent, void *const) noexcept {
    evbuffer_drain(bufferevent_get_input(bufferEvent), evbuffer_get_length(bufferevent_get_input(bufferEvent)));

    static constexpr auto response{
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 0\r\n"
        "\r\n"sv};

    bufferevent_write(bufferEvent, std::data(response), std::size(response));
}

constexpr auto eventAction(bufferevent *const bufferEvent, const std::int16_t events, void *const) noexcept {
    if (events & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) bufferevent_free(bufferEvent);
}

constexpr auto acceptAction(evconnlistener *const listener, const evutil_socket_t socket, sockaddr *const,
                            const std::int32_t, void *const) noexcept {
    bufferevent *const bufferEvent{
        bufferevent_socket_new(evconnlistener_get_base(listener), socket,
                               BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS | BEV_OPT_UNLOCK_CALLBACKS)};

    bufferevent_enable(bufferEvent, EV_READ | EV_WRITE);
    bufferevent_setcb(bufferEvent, receiveAction, nullptr, eventAction, nullptr);
}

constexpr auto acceptErrorAction(evconnlistener *const listener, void *const) noexcept {
    event_base_loopbreak(evconnlistener_get_base(listener));

    evconnlistener_free(listener);
}

constexpr auto addListener(event_base *const base) noexcept {
    sockaddr_in address{};
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);

    evconnlistener_set_error_cb(evconnlistener_new_bind(base, acceptAction, base,
                                                        LEV_OPT_DEFERRED_ACCEPT | LEV_OPT_CLOSE_ON_FREE |
                                                            LEV_OPT_REUSEABLE | LEV_OPT_REUSEABLE_PORT,
                                                        -1, reinterpret_cast<sockaddr *>(std::addressof(address)),
                                                        sizeof(address)),
                                acceptErrorAction);
}

constexpr auto execute() noexcept {
    event_config *const config{event_config_new()};
    event_config_set_flag(config, EVENT_BASE_FLAG_NOLOCK | EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST);

    event_base *const base{event_base_new_with_config(config)};
    event_config_free(config);

    addListener(base);

    event_base_dispatch(base);

    event_base_free(base);
}

auto main() -> int {
    std::vector<std::jthread> workers;
    for (std::uint32_t i{}; i != std::thread::hardware_concurrency() - 1; ++i) workers.emplace_back(execute);

    execute();
}
