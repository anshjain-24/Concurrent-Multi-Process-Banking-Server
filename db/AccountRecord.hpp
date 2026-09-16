#pragma once

namespace bank {

inline constexpr int MAX_TRANSACTIONS = 20;

// NOTE: Kept as a plain, trivially-copyable POD struct (fixed-size arrays,
// no std::string/std::vector) deliberately. The server reads and writes
// these records directly as raw bytes (write(fd, &record, sizeof(record))),
// indexed by account_number as a byte offset into ACCOUNT_FILE. If this
// struct stops being trivially-copyable / standard-layout, that binary
// file format breaks. Same reasoning applies to User and Transaction.
struct Account {
    int account_number;
    int owners[2];       // owners[1] == -1 for a regular (non-joint) account
    int account_type;    // 0 = regular, 1 = joint
    bool active_status;
    long balance;
    int transactions[MAX_TRANSACTIONS]; // ring buffer of transaction IDs, -1 = empty slot
};

} // namespace bank
