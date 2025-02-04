#include "LoggerImpl.hpp"

#include <syncstream>

using namespace std::string_view_literals;

auto coContext::internal::LoggerImpl::Node::operator new(const std::size_t bytes) -> void * {
    return getSyncMemoryResource()->allocate(bytes);
}

auto coContext::internal::LoggerImpl::Node::operator delete(void *const pointer, const std::size_t bytes) noexcept
    -> void {
    getSyncMemoryResource()->deallocate(pointer, bytes);
}

coContext::internal::LoggerImpl::LoggerImpl() {
    this->enableWrite();
    this->run();
}

coContext::internal::LoggerImpl::~LoggerImpl() {
    const Node *node{this->head.load(std::memory_order::relaxed)};
    while (node != nullptr) {
        const Node *const next{node->next};
        delete node;
        node = next;
    }
}

auto coContext::internal::LoggerImpl::run() -> void {
    static constinit std::mutex mutex;

    const std::lock_guard lock{mutex};

    if (!this->worker.joinable()) {
        this->worker = std::jthread{[this](std::stop_token token) constexpr {
            do {
                const Node *node{reverseList(this->head.exchange(nullptr, std::memory_order::relaxed))};
                while (node != nullptr) {
                    std::osyncstream syncOutputStream{*this->outputStream.load(std::memory_order::relaxed)};
                    std::println(syncOutputStream, "{}"sv, node->log);

                    const Node *const next{node->next};
                    delete node;
                    node = next;
                }

                this->notifyVariable.wait(false, std::memory_order::relaxed);
                this->notifyVariable.clear(std::memory_order::relaxed);
            } while (!token.stop_requested());
        }};
    }
}

auto coContext::internal::LoggerImpl::stop() noexcept -> void {
    this->worker.request_stop();

    this->notify();
}

auto coContext::internal::LoggerImpl::setOutputStream(std::ostream *const outputStream) noexcept -> void {
    this->outputStream.store(outputStream, std::memory_order::relaxed);
}

auto coContext::internal::LoggerImpl::enableWrite() noexcept -> void {
    this->writeSwitch.test_and_set(std::memory_order::relaxed);
}

auto coContext::internal::LoggerImpl::disableWrite() noexcept -> void {
    this->writeSwitch.clear(std::memory_order::relaxed);
}

auto coContext::internal::LoggerImpl::getLevel() const noexcept -> Log::Level {
    return this->level.load(std::memory_order::relaxed);
}

auto coContext::internal::LoggerImpl::setLevel(const Log::Level level) noexcept -> void {
    this->level.store(level, std::memory_order::relaxed);
}

auto coContext::internal::LoggerImpl::write(Log log) -> void {
    if (!this->writeSwitch.test(std::memory_order::relaxed) ||
        log.getLevel() < this->level.load(std::memory_order::relaxed))
        return;

    auto *const node{
        new Node{std::move(log), this->head.load(std::memory_order::relaxed)}
    };
    while (!this->head.compare_exchange_weak(node->next, node, std::memory_order::release, std::memory_order::relaxed));

    this->notify();
}

auto coContext::internal::LoggerImpl::flush() const -> void {
    std::osyncstream{*this->outputStream.load(std::memory_order::relaxed)}.flush();
}

auto coContext::internal::LoggerImpl::reverseList(Node *node) noexcept -> Node * {
    Node *previous{};
    while (node != nullptr) {
        Node *const next{node->next};
        node->next = previous;
        previous = node;
        node = next;
    }

    return previous;
}

auto coContext::internal::LoggerImpl::notify() noexcept -> void {
    this->notifyVariable.test_and_set(std::memory_order::relaxed);
    this->notifyVariable.notify_one();
}
