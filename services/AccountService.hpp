#pragma once
#include <optional>
#include "../common/Connection.hpp"
#include "../db/AccountRecord.hpp"

namespace bank {

// Direct translation of get_account_details(). Two modes, matching the
// original's NULL-pointer-vs-not distinction:
//   - presetAccountNumber has a value  -> "silent" internal lookup used by
//     deposit/withdraw/etc: no prompt, no printed summary, just returns
//     the record.
//   - presetAccountNumber is nullopt   -> interactive lookup used by the
//     admin menu: asks the client for an account number and prints a
//     formatted summary.
// Either way, an invalid account number always sends INVALID_ACCOUNT_ID to
// the client (matches the original, which did this unconditionally).
std::optional<Account> getAccountDetails(const Connection& conn, std::optional<int> presetAccountNumber);

// Direct translation of get_user_details(). userId == nullopt prompts the
// client for one (used by the admin menu); otherwise looks the given ID up
// directly (used when a logged-in user views their own details).
void getUserDetails(const Connection& conn, std::optional<int> userId);

// Direct translation of get_transaction_details(). accountNumber ==
// nullopt prompts the client for one (admin menu); otherwise uses the
// given account directly (logged-in user viewing their own history).
void getTransactionDetails(const Connection& conn, std::optional<int> accountNumber);

} // namespace bank
