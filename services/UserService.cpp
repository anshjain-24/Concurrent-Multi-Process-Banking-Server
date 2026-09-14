#include "UserService.hpp"
#include "AccountService.hpp"
#include "LoginService.hpp"
#include "../messages/Messages.hpp"
#include "../common/RecordFile.hpp"
#include "../common/CriticalSection.hpp"
#include "../db/AccountRecord.hpp"
#include "../db/TransactionRecord.hpp"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>

namespace bank {

namespace {

// Direct translation of write_transaction_to_array(): appends a
// transaction id to the account's ring buffer, shifting out the oldest
// entry once it's full.
void pushTransactionId(int (&transactionList)[MAX_TRANSACTIONS], int transactionId) {
    int i = 0;
    while (i < MAX_TRANSACTIONS && transactionList[i] != -1) ++i;

    if (i >= MAX_TRANSACTIONS) {
        for (i = 1; i < MAX_TRANSACTIONS; ++i)
            transactionList[i - 1] = transactionList[i];
        transactionList[MAX_TRANSACTIONS - 1] = transactionId;
    } else {
        transactionList[i] = transactionId;
    }
}

// Direct translation of write_transaction_to_file(): appends a new
// transaction record and returns its id (previous id + 1, or 0 if this is
// the first transaction on file).
int recordTransaction(int accountNumber, long oldBalance, long newBalance, bool isDeposit) {
    RecordFile<Transaction> transactions(msg::TRANSACTION_FILE);

    Transaction t{};
    t.account_number = accountNumber;
    t.old_balance = oldBalance;
    t.new_balance = newBalance;
    t.operation = isDeposit;
    t.transaction_time = std::time(nullptr);

    auto prev = transactions.lastRecord();
    t.transaction_id = prev ? prev->transaction_id + 1 : 0;

    transactions.appendRecord(t);
    return t.transaction_id;
}

void getBalance(const Connection& conn, const User& loggedInUser) {
    auto account = getAccountDetails(conn, loggedInUser.account_number);
    if (!account) return;

    if (account->active_status)
        conn.inform("You have \u20B9 " + std::to_string(account->balance) + "  money in your account!^");
    else
        conn.inform(msg::ACCOUNT_DEACTIVATED_MESSAGE);
}

void deposit(const Connection& conn, const User& loggedInUser, const AccountSemaphore& sem) {
    CriticalSection guard(sem);

    auto account = getAccountDetails(conn, loggedInUser.account_number);
    if (!account) return;

    if (!account->active_status) {
        conn.inform(msg::ACCOUNT_DEACTIVATED_MESSAGE);
        return;
    }

    long amount = std::atol(conn.ask(msg::DEPOSIT_AMOUNT_QUESTION).c_str());
    if (amount <= 0) {
        conn.inform(msg::INVALID_DEPOSIT_AMOUNT);
        return;
    }

    int txId = recordTransaction(account->account_number, account->balance, account->balance + amount, true);
    pushTransactionId(account->transactions, txId);
    account->balance += amount;

    RecordFile<Account> accounts(msg::ACCOUNT_FILE);
    accounts.writeAt(account->account_number, *account);

    conn.inform(msg::DEPOSIT_AMOUNT_SUCCESS);
    getBalance(conn, loggedInUser);
}

void withdraw(const Connection& conn, const User& loggedInUser, const AccountSemaphore& sem) {
    CriticalSection guard(sem);

    auto account = getAccountDetails(conn, loggedInUser.account_number);
    if (!account) return;

    if (!account->active_status) {
        conn.inform(msg::ACCOUNT_DEACTIVATED_MESSAGE);
        return;
    }

    long amount = std::atol(conn.ask(msg::WITHDRAW_AMOUNT_QUESTION).c_str());
    if (amount <= 0 || account->balance - amount < 0) {
        conn.inform(msg::INVALID_WITHDRAW_AMOUNT);
        return;
    }

    int txId = recordTransaction(account->account_number, account->balance, account->balance - amount, false);
    pushTransactionId(account->transactions, txId);
    account->balance -= amount;

    RecordFile<Account> accounts(msg::ACCOUNT_FILE);
    accounts.writeAt(account->account_number, *account);

    conn.inform(msg::WITHDRAW_AMOUNT_SUCCESS);
    getBalance(conn, loggedInUser);
}

void changePassword(const Connection& conn, User& loggedInUser, const AccountSemaphore& sem) {
    CriticalSection guard(sem);

    std::string oldPassword = conn.ask(msg::OLD_PASS_REQUEST);
    if (oldPassword != loggedInUser.password) {
        conn.inform(msg::INVALID_OLD_PASSWORD);
        return;
    }

    std::string newPassword = conn.ask(msg::NEW_PASSWORD_REQUEST);
    std::string confirm = conn.ask(msg::NEW_PASSWORD_RE_ENTER);
    if (confirm != newPassword) {
        conn.inform(msg::PASSWORD_DO_NOT_MATCH);
        return;
    }

    std::strncpy(loggedInUser.password, newPassword.c_str(), sizeof(loggedInUser.password) - 1);

    RecordFile<User> users(msg::USER_FILE);
    users.writeAt(loggedInUser.user_id, loggedInUser);

    conn.inform(msg::PASSWORD_CHANGE_SUCCESS);
}

} // namespace

void runUserSession(const Connection& conn) {
    auto result = login(false, conn);
    if (!result.success) return;

    User loggedInUser = result.user;
    // Same semaphore key scheme as the original: ftok() against a fixed
    // path, keyed by account number, so all forked child processes
    // serving the same account share one binary semaphore.
    AccountSemaphore sem(msg::USER_FILE, loggedInUser.account_number);

    std::string prompt = std::string(msg::CUSTOMER_LOGIN_SUCCESS) + "\n" + msg::USER_MENU;
    while (true) {
        conn.send(prompt);
        int choice = std::atoi(conn.receive().c_str());

        switch (choice) {
            case 1: getUserDetails(conn, loggedInUser.user_id); break;
            case 2: deposit(conn, loggedInUser, sem); break;
            case 3: withdraw(conn, loggedInUser, sem); break;
            case 4: getBalance(conn, loggedInUser); break;
            case 5: getTransactionDetails(conn, loggedInUser.account_number); break;
            case 6: changePassword(conn, loggedInUser, sem); break;
            default:
                conn.send(msg::USER_LOGOUT);
                return;
        }

        prompt = std::string("\n") + msg::USER_MENU;
    }
}

} // namespace bank
