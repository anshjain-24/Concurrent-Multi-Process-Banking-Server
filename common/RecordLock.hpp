#pragma once
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>

namespace bank {

// Obtains an fcntl() byte-range lock (F_RDLCK or F_WRLCK) on construction
// and releases it on destruction. Directly replaces the
// "build struct flock; fcntl(F_SETLKW); ... ; lock.l_type = F_UNLCK;
// fcntl(F_SETLK)" pattern that appears ~20 times across the original
// admin_utility.h / user_utility.h / common_utility.h -- and, same as
// FileDescriptor, removes the risk of an early return skipping the unlock.
class RecordLock {
public:
    RecordLock(int fd, short type, off_t offset, off_t length) : fd_(fd) {
        lock_.l_type = type;
        lock_.l_whence = SEEK_SET;
        lock_.l_start = offset;
        lock_.l_len = length;
        lock_.l_pid = getpid();
        locked_ = (fcntl(fd_, F_SETLKW, &lock_) != -1);
        if (!locked_) perror("Error obtaining record lock");
    }

    RecordLock(const RecordLock&) = delete;
    RecordLock& operator=(const RecordLock&) = delete;

    ~RecordLock() {
        if (locked_) {
            lock_.l_type = F_UNLCK;
            fcntl(fd_, F_SETLK, &lock_);
        }
    }

    bool ok() const { return locked_; }

private:
    int fd_;
    struct flock lock_{};
    bool locked_ = false;
};

} // namespace bank
