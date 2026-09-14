#pragma once

// Direct C++ translation of error_message.h + general_message.h.
// Kept as free constants (not an enum/class) since they're used purely as
// text sent over the socket -- no behavioural difference from the C
// #defines, just type-safe and scoped under bank::msg instead of polluting
// the global namespace.
//
// Protocol reminder (see Client.cpp for the receiving side):
//   trailing '^'  -> client prints the message, then sends a dummy ack
//   trailing '$'  -> client prints the message, then the connection closes
//   a '#' inside  -> client should read the reply with echo disabled (password)

namespace bank::msg {

// ---- errors ----
inline constexpr const char* SOCKET_ERROR = "Error while creating server socket.";
inline constexpr const char* BIND_ERROR = "Error while binding to server socket.";
inline constexpr const char* LISTEN_ERROR = "Error while listening connection to server socket.";
inline constexpr const char* CLIENT_CONNECTION_ERROR = "Error while accepting connection from client.";
inline constexpr const char* SERVER_CONNECTION_ERROR = "Error while connecting to the server.";
inline constexpr const char* WELCOME_PROMPT_ERROR = "Error while sending welcome prompt to user.";
inline constexpr const char* CLIENT_READ_ERROR = "Error while reading from the client.";
inline constexpr const char* SERVER_READ_ERROR = "Error while reading from the server.";
inline constexpr const char* EMPTY_INPUT_ERROR = "No data sent by the user.";
inline constexpr const char* EMPTY_RESPONSE_ERROR = "No response from the server";
inline constexpr const char* CLIENT_WRITE_ERROR = "Error while writing to client socket";
inline constexpr const char* FILE_OPEN_ERROR = "Error while opening file";
inline constexpr const char* INVALID_USER_ID = "Invalid user id , user does not exist\n ^";
inline constexpr const char* INVALID_PASSWORD = "Invalid password $";
inline constexpr const char* INVALID_GENDER = "You've enter a wrong gender choice!\n Returning to the main menu!^";
inline constexpr const char* INVALID_AGE = "You've enter a  invalid age!\n Returning to the main menu!^";
inline constexpr const char* TRANSACTIONS_NOT_FOUND = "No transactions were performed on this account by the customer!^";
inline constexpr const char* INVALID_DEPOSIT_AMOUNT = "Invalid deposit amount entered!^";
inline constexpr const char* INVALID_WITHDRAW_AMOUNT = "Invalid withdraw amount entered!^";
inline constexpr const char* INVALID_OLD_PASSWORD = "Invalid old password";
inline constexpr const char* PASSWORD_DO_NOT_MATCH = "The new password and the reentered passwords do not match!^";
inline constexpr const char* INVALID_OPTION = "Invalid option choosen \n$";
inline constexpr const char* INVALID_ACCOUNT_ID = "ACCOUNT ID DOES NOT EXIST";

// ---- general / menu text ----
inline constexpr const char* WELCOME_PROMPT =
    "Welcome to Clix bank !!\nWho are you?\n1. Admin\t2. User \nEnter the number corresponding to the choice!\nPress any other number to exit thanks !!\n";
inline constexpr const char* LOGIN_PROMPT = "Welcome ! Enter your credentials to login to your account! \nEnter your login ID";
inline constexpr const char* EXIT_PROMPT = "Thanks for the connecting , hoping to see you soon";
inline constexpr const char* CLOSING_CONNECTION = "Closing connection for now\n$";
inline constexpr const char* ADMIN_LOGIN_SUCCESS = "Login Successfull , Welcome Admin";
inline constexpr const char* CUSTOMER_LOGIN_SUCCESS = "Login Successfull, Weclome User!";
inline constexpr const char* PASSWORD_REQUEST = "Enter your password \n# ";
inline constexpr const char* CLIENT_CONNECT_SUCCESS = "Client has sucessfully connected to the server";
inline constexpr const char* ADMIN_PROMPT =
    "1. Add Account \n2. Get Account Details\n3. Get Transaction details\n4. Get User Details \n5. Delete Account\n6. Modify User Information\nEnter the number corresponding to the choice!\nPress any other key to logout";
inline constexpr const char* ADMIN_ACCOUNT_TYPE = "Enter type of account to create? Enter 0 for regular account and 1 for joint account";
inline constexpr const char* USER_GENDER_QUESTION = "Enter gender of user? \n Enter M for male , F for female and O for others";
inline constexpr const char* USER_AGE_QUESTION = "Enter the age of user";
inline constexpr const char* ACCOUNT_NUMBER_QUESTION = "Enter the Account Number to get the details";
inline constexpr const char* USER_ID_QUESTION = "Enter the User ID to get the details";
inline constexpr const char* USER_MENU =
    "1. Get Customer Details\n2. Deposit Money\n3. Withdraw Money\n4. Get Balance\n5. Get Transaction information\n6. Change Password\nPress any other key to logout";
inline constexpr const char* DEPOSIT_AMOUNT_QUESTION = "Enter Amount to be deposited ?";
inline constexpr const char* DEPOSIT_AMOUNT_SUCCESS = "Amount has been deposited successfully!^";
inline constexpr const char* ACCOUNT_DEACTIVATED_MESSAGE = "The given account has been deactivated!^";
inline constexpr const char* WITHDRAW_AMOUNT_QUESTION = "Enter the amount you want to wthdraw ?";
inline constexpr const char* WITHDRAW_AMOUNT_SUCCESS = "Amount has been withdrawed successfully!^";
inline constexpr const char* OLD_PASS_REQUEST = "Enter your old password";
inline constexpr const char* NEW_PASSWORD_REQUEST = "Enter the new password";
inline constexpr const char* NEW_PASSWORD_RE_ENTER = "Reenter the new password";
inline constexpr const char* PASSWORD_CHANGE_SUCCESS = "Password successfully changed!^";
inline constexpr const char* USER_LOGOUT = "Logging out ! Good bye!$";
inline constexpr const char* USER_FILE = "./db/USER_FILE";
inline constexpr const char* ACCOUNT_FILE = "./db/ACCOUNT_FILE";
inline constexpr const char* TRANSACTION_FILE = "./db/TRANSACTION_FILE"; // was a literal, not a macro, in the original
inline constexpr const char* DELETE_ACCOUNT_NUMBER_QUESTION = "Enter account number to deleted";
inline constexpr const char* MODIFY_USER_ID_QUESTION = "Enter user id to modify the information";
inline constexpr const char* ACCOUNT_DELETE_SUCCESS = "Account deleted succesfully";
inline constexpr const char* MODIFY_INFORMATION_OPTION = "Which information would you like to modify?\n1. Name 2. Age 3. Gender \nPress any other key to cancel";
inline constexpr const char* MODIFY_NAME_MESSAGE = "Enter new name to modify!";
inline constexpr const char* MODIFY_GENDER_MESSAGE = "Enter new gender to modify !";
inline constexpr const char* MODIFY_AGE_MESSAGE = "Enter new age to modify!";
inline constexpr const char* MODIFY_SUCCESS_MESSAGE = "User information upadted successfully!^";
inline constexpr const char* ADMIN_LOGOUT = "Logging out admin $";

} // namespace bank::msg
