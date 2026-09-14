#pragma once
#include <ctime>

namespace bank {

struct Transaction {
    int transaction_id;
    int account_number;
    bool operation;   // false = withdraw, true = deposit
    long old_balance;
    long new_balance;
    time_t transaction_time;
};

} // namespace bank
