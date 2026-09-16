#pragma once
#include <ctime>

namespace bank {

// See the comment in AccountRecord.hpp -- deliberately POD, mirrors the
// original C struct field-for-field so USER_FILE stays byte-compatible.
struct User {
    int user_id;
    char user_name[30];
    int account_number;
    char login_id[40];   // "<name>-<user_id>", e.g. "Alice-3"
    char password[30];
    char gender;
    int age;
    time_t account_opening_date;
};

} // namespace bank
