#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <termios.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

#include "messages/Messages.hpp"
#include "common/Connection.hpp"
#include "common/FileDescriptor.hpp"

namespace {

// Reads a line from stdin with terminal echo disabled -- replaces the
// original's getpass(), which is deprecated/removed on several modern
// libc implementations (glibc still has it, but it's not portable to
// e.g. macOS's libc going forward). Same user-facing behaviour: print the
// prompt, read a line without echoing it.
std::string readHidden(const std::string& prompt) {
    std::cout << prompt;
    std::cout.flush();

    termios oldSettings{}, newSettings{};
    bool haveTty = (tcgetattr(STDIN_FILENO, &oldSettings) == 0);
    if (haveTty) {
        newSettings = oldSettings;
        newSettings.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);
    }

    std::string input;
    std::getline(std::cin, input);

    if (haveTty) {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
        std::cout << "\n";
    }
    return input;
}

// Drives the same request/response loop as the original connection_handler():
// reads a message from the server, and depending on which sentinel
// character it ends with, either prints-and-acks ('^'), prints-and-closes
// ('$'), or treats it as a prompt (reading a plain or hidden reply and
// sending it back, depending on whether the prompt contains '#').
void runProtocolLoop(bank::Connection& conn) {
    while (true) {
        std::string message = conn.receive();
        if (message.empty()) {
            std::printf("%s\n", bank::msg::EMPTY_RESPONSE_ERROR);
            break;
        }

        if (message.find('^') != std::string::npos) {
            std::cout << message.substr(0, message.size() >= 1 ? message.size() - 1 : 0) << "\n";
            if (!conn.send("^")) {
                perror(bank::msg::CLIENT_WRITE_ERROR);
                break;
            }
        } else if (message.find('$') != std::string::npos) {
            std::cout << message.substr(0, message.size() >= 2 ? message.size() - 2 : 0) << "\n";
            std::printf("%s", bank::msg::CLOSING_CONNECTION);
            break;
        } else {
            std::string reply = (message.find('#') != std::string::npos)
                ? readHidden(message)
                : [&] {
                    std::cout << message << "\n";
                    std::string line;
                    std::getline(std::cin, line);
                    return line;
                  }();

            if (!conn.send(reply)) {
                perror(bank::msg::CLIENT_WRITE_ERROR);
                std::printf("%s", bank::msg::CLOSING_CONNECTION);
                break;
            }
        }
    }
}

} // namespace

int main() {
    bank::FileDescriptor socketFd(socket(AF_INET, SOCK_STREAM, 0));
    if (!socketFd) {
        perror(bank::msg::SOCKET_ERROR);
        return 1;
    }

    struct sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // matches the original -- see README for why this should really be a real server address
    serverAddress.sin_port = htons(4002);

    if (connect(socketFd.get(), reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        perror(bank::msg::SERVER_CONNECTION_ERROR);
        return 1;
    }

    bank::Connection conn(socketFd.get());
    runProtocolLoop(conn);

    return 0;
}
