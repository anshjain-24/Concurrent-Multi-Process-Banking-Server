#pragma once
#include "../common/Connection.hpp"

namespace bank {

// Direct translation of user_operation_handler(): logs the customer in,
// sets up their per-account semaphore, then loops the customer menu until
// they pick an unrecognised option (logout).
void runUserSession(const Connection& conn);

} // namespace bank
