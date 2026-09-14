#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "messages/Messages.hpp"
#include "common/Connection.hpp"
#include "common/FileDescriptor.hpp"
#include "services/AdminService.hpp"
#include "services/UserService.hpp"

namespace {

// Direct translation of connection_handler(): sends the welcome prompt,
// reads the client's admin/user choice, and dispatches accordingly.
void handleConnection(bank::Connection conn) {
    std::printf("%s\n", bank::msg::CLIENT_CONNECT_SUCCESS);

    if (!conn.send(bank::msg::WELCOME_PROMPT)) {
        perror(bank::msg::WELCOME_PROMPT_ERROR);
        return;
    }

    std::string choice = conn.receive();
    if (choice.empty()) {
        perror(bank::msg::EMPTY_INPUT_ERROR);
        return;
    }

    switch (std::atoi(choice.c_str())) {
        case 1: bank::runAdminSession(conn); break;
        case 2: bank::runUserSession(conn); break;
        default: conn.send(bank::msg::INVALID_OPTION); break;
    }

    std::printf("%s", bank::msg::CLOSING_CONNECTION);
}

} // namespace

int main() {
    bank::FileDescriptor listener(socket(AF_INET, SOCK_STREAM, 0));
    if (!listener) {
        perror(bank::msg::SOCKET_ERROR);
        return 1;
    }

    struct sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(4002);

    if (bind(listener.get(), reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        perror(bank::msg::BIND_ERROR);
        return 1;
    }

    if (listen(listener.get(), 10) == -1) {
        perror(bank::msg::LISTEN_ERROR);
        return 1;
    }

    while (true) {
        struct sockaddr_in clientAddress{};
        socklen_t clientSize = sizeof(clientAddress);

        int connectionFd = accept(listener.get(), reinterpret_cast<struct sockaddr*>(&clientAddress), &clientSize);
        if (connectionFd == -1) {
            perror(bank::msg::CLIENT_CONNECTION_ERROR);
            continue;
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("Error forking connection handler");
            close(connectionFd);
            continue;
        }
        if (pid == 0) {
            // Child: listener fd isn't needed here, but we deliberately
            // don't bother closing it -- it's about to _exit() anyway.
            handleConnection(bank::Connection(connectionFd));
            close(connectionFd);
            _exit(0);
        }
        // Parent: done with this connection, the child owns it now.
        close(connectionFd);
    }
}
