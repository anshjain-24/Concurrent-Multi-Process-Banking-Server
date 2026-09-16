#include "AccountService.hpp"
#include "../messages/Messages.hpp"
#include "../common/RecordFile.hpp"
#include "../db/UserRecord.hpp"
#include "../db/TransactionRecord.hpp"
#include <cstdlib>
#include <ctime>
#include <string>

namespace bank {

std::optional<Account> getAccountDetails(const Connection& conn, std::optional<int> presetAccountNumber) {
    int accountNumber = presetAccountNumber
        ? *presetAccountNumber
        : std::atoi(conn.ask(msg::ACCOUNT_NUMBER_QUESTION).c_str());

    RecordFile<Account> accounts(msg::ACCOUNT_FILE);
    auto record = accounts.readAt(accountNumber);
    if (!record) {
        conn.inform(std::string(msg::INVALID_ACCOUNT_ID) + "^");
        return std::nullopt;
    }

    if (presetAccountNumber) {
        // Silent lookup for internal callers -- just hand the record back.
        return record;
    }

    // Interactive lookup (admin menu) -- format and print a summary,
    // reproducing the original sprintf-built message.
    std::string summary = "Account Details - \n\tAccount Number : " + std::to_string(record->account_number) +
        "\n\tAccount Type : " + (record->account_type ? "Joint" : "Regular") +
        "\n\tAccount Status : " + (record->active_status ? "Active" : "Deactived");
    if (record->active_status)
        summary += "\n\tAccount Balance:\u20B9 " + std::to_string(record->balance); // "\u20B9" == ₹
    summary += "\n\tPrimary Owner ID: " + std::to_string(record->owners[0]);
    if (record->owners[1] != -1)
        summary += "\n\tSecondary Owner ID: " + std::to_string(record->owners[1]);
    summary += "\n^";

    conn.inform(summary);
    return record;
}

void getUserDetails(const Connection& conn, std::optional<int> userId) {
    int id = userId ? *userId : std::atoi(conn.ask(msg::USER_ID_QUESTION).c_str());

    RecordFile<User> users(msg::USER_FILE);
    auto record = users.readAt(id);
    if (!record) {
        conn.inform(std::string(msg::INVALID_USER_ID) + "^");
        return;
    }

    struct tm openedAt = *std::localtime(&record->account_opening_date);
    std::string summary = "user Details - \n\tUser ID : " + std::to_string(record->user_id) +
        "\n\tName : " + record->user_name +
        "\n\tGender : " + std::string(1, record->gender) +
        "\n\tAge: " + std::to_string(record->age) +
        "\n\tAccount Number : " + std::to_string(record->account_number) +
        "\n\tLogin ID : " + record->login_id +
        "\n\tAccount Opening Date:-  " + std::to_string(openedAt.tm_hour) + ":" + std::to_string(openedAt.tm_min) +
        " " + std::to_string(openedAt.tm_mday) + "/" + std::to_string(openedAt.tm_mon + 1) + "/" +
        std::to_string(openedAt.tm_year + 1900) +
        "\n\nYou'll now be redirected to the main menu...^";

    conn.inform(summary);
}

void getTransactionDetails(const Connection& conn, std::optional<int> accountNumber) {
    int number = accountNumber ? *accountNumber : std::atoi(conn.ask(msg::ACCOUNT_NUMBER_QUESTION).c_str());

    auto account = getAccountDetails(conn, number);
    if (!account) return; // getAccountDetails already informed the client

    RecordFile<Transaction> transactions(msg::TRANSACTION_FILE);
    std::string summary;

    for (int i = 0; i < MAX_TRANSACTIONS && account->transactions[i] != -1; ++i) {
        auto transaction = transactions.readAt(account->transactions[i]);
        if (!transaction) continue;

        struct tm when = *std::localtime(&transaction->transaction_time);
        summary += "Details of transaction " + std::to_string(i + 1) + " - \n\t Date : " +
            std::to_string(when.tm_hour) + ":" + std::to_string(when.tm_min) + " " +
            std::to_string(when.tm_mday) + "/" + std::to_string(when.tm_mon + 1) + "/" +
            std::to_string(when.tm_year + 1900) + " \n\t Operation : " +
            (transaction->operation ? "Deposit" : "Withdraw") + " \n\t Balance - \n\t\t Before : " +
            std::to_string(transaction->old_balance) + " \n\t\t After : " +
            std::to_string(transaction->new_balance) + " \n\t\t Difference : " +
            std::to_string(transaction->new_balance - transaction->old_balance) + "\n";
    }

    if (summary.empty()) {
        conn.inform(msg::TRANSACTIONS_NOT_FOUND);
    } else {
        summary += "^";
        conn.inform(summary);
    }
}

} // namespace bank
