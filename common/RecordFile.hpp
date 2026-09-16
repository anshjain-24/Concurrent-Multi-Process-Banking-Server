#pragma once
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <optional>
#include "FileDescriptor.hpp"
#include "RecordLock.hpp"

namespace bank {

// The original C code treats each db file (ACCOUNT_FILE, USER_FILE,
// TRANSACTION_FILE) as a flat array of fixed-size structs on disk: record
// N lives at byte offset N * sizeof(Record), located with lseek() and
// protected with an fcntl() lock for the duration of the read/write.
// That exact pattern -- open, seek, lock, read-or-write, unlock, close --
// is duplicated by hand well over a dozen times across the original
// utility headers. RecordFile<T> is that pattern, written once.
//
// Deletion is soft (same as the original): records are never removed or
// compacted, callers just flip a status flag (e.g. Account::active_status)
// and rewrite the record in place.
template <typename Record>
class RecordFile {
public:
    explicit RecordFile(const char* path) : path_(path) {}

    // Reads the record at index `idx`. Returns std::nullopt if the file
    // doesn't exist yet or the index is out of range.
    std::optional<Record> readAt(int idx) const {
        FileDescriptor fd(open(path_, O_RDONLY));
        if (!fd) return std::nullopt;

        off_t offset = lseek(fd.get(), static_cast<off_t>(idx) * sizeof(Record), SEEK_SET);
        if (offset == -1) return std::nullopt;

        RecordLock lock(fd.get(), F_RDLCK, offset, sizeof(Record));
        if (!lock.ok()) return std::nullopt;

        Record record{};
        if (read(fd.get(), &record, sizeof(Record)) != static_cast<ssize_t>(sizeof(Record)))
            return std::nullopt;

        return record;
    }

    // Overwrites the record at index `idx` in place. The file (and that
    // record slot) must already exist -- use appendRecord() for brand new
    // records.
    bool writeAt(int idx, const Record& record) const {
        FileDescriptor fd(open(path_, O_WRONLY));
        if (!fd) return false;

        off_t offset = lseek(fd.get(), static_cast<off_t>(idx) * sizeof(Record), SEEK_SET);
        if (offset == -1) return false;

        RecordLock lock(fd.get(), F_WRLCK, offset, sizeof(Record));
        if (!lock.ok()) return false;

        return write(fd.get(), &record, sizeof(Record)) == static_cast<ssize_t>(sizeof(Record));
    }

    // Appends a new record to the end of the file, creating the file if
    // it doesn't exist yet. Returns the index it was written to, or -1 on
    // failure.
    int appendRecord(const Record& record) const {
        FileDescriptor fd(open(path_, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU));
        if (!fd) return -1;
        if (write(fd.get(), &record, sizeof(Record)) != static_cast<ssize_t>(sizeof(Record)))
            return -1;
        return recordCount() - 1;
    }

    // Number of records currently in the file (0 if it doesn't exist).
    int recordCount() const {
        FileDescriptor fd(open(path_, O_RDONLY));
        if (!fd) return 0;
        off_t size = lseek(fd.get(), 0, SEEK_END);
        if (size <= 0) return 0;
        return static_cast<int>(size / sizeof(Record));
    }

    // Convenience: the last record in the file, if any.
    std::optional<Record> lastRecord() const {
        int count = recordCount();
        if (count == 0) return std::nullopt;
        return readAt(count - 1);
    }

private:
    const char* path_;
};

} // namespace bank
