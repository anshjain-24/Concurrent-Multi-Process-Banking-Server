#pragma once
#include "../common/Connection.hpp"

namespace bank {

// Direct translation of admin_operation_handler(): logs the admin in,
// then loops the admin menu until they pick an unrecognised option
// (logout).
void runAdminSession(const Connection& conn);

} // namespace bank
