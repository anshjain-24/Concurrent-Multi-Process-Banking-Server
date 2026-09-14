#include "LoginService.hpp"
#include "../messages/Messages.hpp"
#include "../db/AdminCredentials.hpp"
#include "../common/RecordFile.hpp"
#include <cstdlib>
#include <cstring>

namespace bank {

LoginResult login(bool isAdmin, const Connection& conn) {
    LoginResult result;

    std::string loginId = conn.ask(msg::LOGIN_PROMPT);

    if (isAdmin) {
        if (loginId != ADMIN_LOGIN_ID) {
            conn.send(msg::INVALID_USER_ID);
            return result;
        }
    } else {
        // login_id format is "<name>-<user_id>" -- find the first '-' the
        // same way the original strtok(buf, "-") + strtok(NULL, "-") did.
        auto dash = loginId.find('-');
        if (dash == std::string::npos) {
            conn.send(msg::INVALID_USER_ID);
            return result;
        }
        int userId = std::atoi(loginId.c_str() + dash + 1);

        RecordFile<User> users(msg::USER_FILE);
        auto record = users.readAt(userId);
        if (!record || std::strncmp(record->login_id, loginId.c_str(), sizeof(record->login_id)) != 0) {
            conn.send(msg::INVALID_USER_ID);
            return result;
        }
        result.user = *record;
    }

    std::string password = conn.ask(msg::PASSWORD_REQUEST);

    if (isAdmin) {
        if (password != ADMIN_PASSWORD) {
            conn.send(msg::INVALID_PASSWORD);
            return result;
        }
    } else {
        if (password != result.user.password) {
            conn.send(msg::INVALID_PASSWORD);
            result.user = User{};
            return result;
        }
    }

    result.success = true;
    return result;
}

} // namespace bank
