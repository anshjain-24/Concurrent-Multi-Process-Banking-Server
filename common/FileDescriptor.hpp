#pragma once
#include <unistd.h>

namespace bank {

// Owns a single POSIX file descriptor (works for both regular files and
// sockets -- they're just ints on POSIX). Closes it automatically on
// destruction, move-only. This is the piece that was missing from the
// original C: every close(fd) had to be typed out by hand, and it's easy
// to forget one on an early-return error path. Here it's not optional.
class FileDescriptor {
public:
    FileDescriptor() = default;
    explicit FileDescriptor(int fd) : fd_(fd) {}

    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    FileDescriptor(FileDescriptor&& other) noexcept : fd_(other.fd_) { other.fd_ = -1; }

    FileDescriptor& operator=(FileDescriptor&& other) noexcept {
        if (this != &other) {
            reset();
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

    ~FileDescriptor() { reset(); }

    int get() const { return fd_; }
    bool valid() const { return fd_ >= 0; }
    explicit operator bool() const { return valid(); }

    void reset(int fd = -1) {
        if (fd_ >= 0) ::close(fd_);
        fd_ = fd;
    }

    // Releases ownership without closing -- used when handing the fd off
    // to something else that will own its lifetime instead (e.g. a forked
    // child process keeping the connection socket alive on its own).
    int release() {
        int tmp = fd_;
        fd_ = -1;
        return tmp;
    }

private:
    int fd_ = -1;
};

} // namespace bank
