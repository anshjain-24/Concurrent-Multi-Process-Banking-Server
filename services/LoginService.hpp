#pragma once
#include "../common/Connection.hpp"
#include "../db/UserRecord.hpp"

namespace bank {

struct LoginResult {
    bool success = false;
    User user{}; // populated only for successful non-admin logins
};

// Direct translation of login_handler(). Prompts for a login ID and
// password over `conn`. For admin logins, checks against the static admin
// credentials. For regular/joint users, the login ID is expected in
// "<name>-<user_id>" form (as produced by AdminService::addUser); the
// numeric suffix is used to look the record up directly in USER_FILE, and
// the stored login_id/password are checked against what was typed.
LoginResult login(bool isAdmin, const Connection& conn);

} // namespace bank
