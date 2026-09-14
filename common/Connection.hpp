#pragma once
#include <string>
#include <unistd.h>

namespace bank {

// Thin wrapper around a connected socket fd implementing the same text
// protocol as the original: send()/receive() are raw write()/read(), and
// any protocol sentinel ('^' = print + dummy ack, '$' = final message,
// '#' = password prompt) is expected to already be embedded in the
// message text, exactly as it was in the original #define strings -- this
// class doesn't reinterpret the protocol, it just removes the repeated
// "write(); if (== -1) { perror(); return false; }" boilerplate that
// wrapped nearly every read/write call in the C version.
class Connection {
public:
    explicit Connection(int fd) : fd_(fd) {}

    bool send(const std::string& text) const {
        ssize_t n = ::write(fd_, text.c_str(), text.size());
        if (n == -1) perror("Error while writing to client socket");
        return n != -1;
    }

    // Reads whatever the peer sends next and returns it as a string
    // (empty string on error or a closed connection).
    std::string receive() const {
        char buffer[1000] = {};
        ssize_t n = ::read(fd_, buffer, sizeof(buffer) - 1);
        if (n <= 0) return {};
        return std::string(buffer, static_cast<size_t>(n));
    }

    // send() a prompt, then receive() the reply -- the extremely common
    // "ask a question, read the answer" round trip.
    std::string ask(const std::string& prompt) const {
        send(prompt);
        return receive();
    }

    // For messages that already end in the '^' sentinel: send them, then
    // perform the client's dummy acknowledgement read (discarded).
    void inform(const std::string& messageEndingInSentinel) const {
        send(messageEndingInSentinel);
        receive();
    }

    int fd() const { return fd_; }

private:
    int fd_;
};

} // namespace bank
