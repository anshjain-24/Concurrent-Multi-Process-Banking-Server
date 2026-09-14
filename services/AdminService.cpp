#include "AdminService.hpp"
#include "AccountService.hpp"
#include "LoginService.hpp"
#include "../messages/Messages.hpp"
#include "../common/RecordFile.hpp"
#include "../db/AccountRecord.hpp"
#include "../db/UserRecord.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <optional>
#include <string>

namespace bank {

namespace {

// Direct translation of add_user(). `isSecondJointOwner` corresponds to
// the original's is_joint_account parameter, which is only ever passed
// true for the *second* owner on a joint account -- the first owner of
// any account (joint or not) is always created with it false.
int addUser(const Connection& conn, bool isSecondJointOwner, int newAccountNumber) {
    RecordFile<User> users(msg::USER_FILE);
    User newUser{};
    newUser.user_id = users.recordCount();

    std::string name = conn.ask(isSecondJointOwner
        ? "Enter details of second user \n Enter name of the user :"
        : "Enter details of  user \n Enter name of the user :");
    std::strncpy(newUser.user_name, name.c_str(), sizeof(newUser.user_name) - 1);

    std::string genderAnswer = conn.ask(msg::USER_GENDER_QUESTION);
    if (genderAnswer.empty() || (genderAnswer[0] != 'M' && genderAnswer[0] != 'F' && genderAnswer[0] != 'O')) {
        conn.inform(msg::INVALID_GENDER);
        return -1;
    }
    newUser.gender = genderAnswer[0];

    int age = std::atoi(conn.ask(msg::USER_AGE_QUESTION).c_str());
    if (age <= 0) {
        conn.inform(msg::INVALID_AGE);
        return -1;
    }
    newUser.age = age;

    std::string password;
    if (!isSecondJointOwner) {
        password = conn.ask(msg::NEW_PASSWORD_REQUEST);
        std::string confirm = conn.ask(msg::NEW_PASSWORD_RE_ENTER);
        if (confirm != password) {
            // Original just perror()'d and returned -1 without notifying
            // the client -- preserved as-is.
            std::fprintf(stderr, "%s\n", msg::PASSWORD_DO_NOT_MATCH);
            return -1;
        }
    } else {
        // Pre-existing quirk carried over from the original: the second
        // owner on a joint account gets a fixed placeholder password
        // ("hellow") instead of being asked to set one. Worth revisiting,
        // but preserved here rather than silently "fixed".
        password = "hellow";
    }

    newUser.account_number = newAccountNumber;
    std::string loginId = name + "-" + std::to_string(newUser.user_id);
    std::strncpy(newUser.login_id, loginId.c_str(), sizeof(newUser.login_id) - 1);
    std::strncpy(newUser.password, password.c_str(), sizeof(newUser.password) - 1);
    newUser.account_opening_date = std::time(nullptr);

    if (users.appendRecord(newUser) == -1) {
        perror("Error while writing new user record");
        return -1;
    }

    conn.inform("The new user id is: " + name + "-" + std::to_string(newUser.user_id) +
        "\nThe password for new user is please save for further login :- " + password + "^");

    return newUser.user_id;
}

bool addAccount(const Connection& conn) {
    RecordFile<Account> accounts(msg::ACCOUNT_FILE);
    Account newAccount{};
    newAccount.account_number = accounts.recordCount();

    std::string typeAnswer = conn.ask(msg::ADMIN_ACCOUNT_TYPE);
    if (typeAnswer.empty() || (typeAnswer[0] != '0' && typeAnswer[0] != '1')) {
        conn.send(msg::INVALID_OPTION);
        return false;
    }
    newAccount.account_type = (typeAnswer[0] == '1') ? 1 : 0;

    newAccount.owners[0] = addUser(conn, /*isSecondJointOwner=*/false, newAccount.account_number);
    if (newAccount.owners[0] == -1) return false;

    newAccount.owners[1] = newAccount.account_type
        ? addUser(conn, /*isSecondJointOwner=*/true, newAccount.account_number)
        : -1;

    newAccount.active_status = true;
    newAccount.balance = 0;
    for (int& t : newAccount.transactions) t = -1;

    if (accounts.appendRecord(newAccount) == -1) {
        perror(msg::FILE_OPEN_ERROR);
        return false;
    }

    conn.inform("The newly created account's number is :" + std::to_string(newAccount.account_number) +
        "\nReturning to the main menu ...^");
    return true;
}

bool deleteAccount(const Connection& conn) {
    int accountNumber = std::atoi(conn.ask(msg::DELETE_ACCOUNT_NUMBER_QUESTION).c_str());

    RecordFile<Account> accounts(msg::ACCOUNT_FILE);
    auto account = accounts.readAt(accountNumber);
    if (!account) {
        conn.inform(std::string(msg::INVALID_ACCOUNT_ID) + "^");
        return false;
    }

    if (account->balance == 0) {
        account->active_status = false;
        accounts.writeAt(accountNumber, *account);
    }
    // Pre-existing quirk: ACCOUNT_DELETE_SUCCESS is sent even when the
    // balance wasn't zero and the account therefore wasn't deactivated
    // (the original had the same behaviour on both branches). Preserved.
    conn.inform(msg::ACCOUNT_DELETE_SUCCESS);
    return true;
}

bool modifyUserInfo(const Connection& conn) {
    int userId = std::atoi(conn.ask(msg::MODIFY_USER_ID_QUESTION).c_str());

    RecordFile<User> users(msg::USER_FILE);
    auto user = users.readAt(userId);
    if (!user) {
        conn.inform(std::string(msg::INVALID_USER_ID) + "^");
        return false;
    }

    int choice = std::atoi(conn.ask(msg::MODIFY_INFORMATION_OPTION).c_str());
    if (choice == 0) {
        conn.send(msg::INVALID_OPTION);
        return false;
    }

    switch (choice) {
        case 1: {
            std::string name = conn.ask(msg::MODIFY_NAME_MESSAGE);
            std::strncpy(user->user_name, name.c_str(), sizeof(user->user_name) - 1);
            break;
        }
        case 2: {
            int age = std::atoi(conn.ask(msg::MODIFY_AGE_MESSAGE).c_str());
            if (age == 0) {
                conn.send(msg::INVALID_OPTION);
                return false;
            }
            user->age = age;
            break;
        }
        case 3: {
            std::string gender = conn.ask(msg::MODIFY_GENDER_MESSAGE);
            if (!gender.empty()) user->gender = gender[0];
            break;
        }
        default:
            conn.send(msg::INVALID_OPTION);
            return false;
    }

    if (!users.writeAt(userId, *user)) {
        perror("Error while writing updated user info into file");
    }

    conn.inform(msg::MODIFY_SUCCESS_MESSAGE);
    return true;
}

} // namespace

void runAdminSession(const Connection& conn) {
    auto result = login(true, conn);
    if (!result.success) return;

    std::string prompt = std::string(msg::ADMIN_LOGIN_SUCCESS) + "\n" + msg::ADMIN_PROMPT;
    while (true) {
        conn.send(prompt);
        int option = std::atoi(conn.receive().c_str());

        switch (option) {
            case 1: addAccount(conn); break;
            case 2: getAccountDetails(conn, std::nullopt); break;
            case 3: getTransactionDetails(conn, std::nullopt); break;
            case 4: getUserDetails(conn, std::nullopt); break;
            case 5: deleteAccount(conn); break;
            case 6: modifyUserInfo(conn); break;
            default:
                conn.send(msg::ADMIN_LOGOUT);
                return;
        }

        prompt = std::string("\n") + msg::ADMIN_PROMPT;
    }
}

} // namespace bank
