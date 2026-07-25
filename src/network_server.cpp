#include "network_server.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include "command_handler.h"
#include "command_parser.h"
#include "key_value_store.h"

namespace
{
void send_response(int client_fd, const std::string& message)
{
    size_t total_sent = 0;
    const size_t message_size = message.size();
    
    while (total_sent < message_size)
    {
        const ssize_t sent = send(client_fd, message.c_str() + total_sent, 
                                  message_size - total_sent, 0);
        if (sent < 0)
        {
            return;
        }
        total_sent += sent;
    }
}
} // namespace

int run_tcp_server(KeyValueStore& store, int port)
{
    constexpr int kBacklog = 10;

    const int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0)
    {
        std::cerr << "Failed to bind socket\n";
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, kBacklog) < 0)
    {
        std::cerr << "Failed to listen\n";
        close(server_fd);
        return 1;
    }

    std::cout << "Redis-like server listening on port " << port << "\n";

    CommandParser parser;
    CommandHandler handler(store);

    while (true)
    {
        const int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0)
        {
            continue;
        }

        std::string line;
        char buffer[1024];

        while (true)
        {
            // Process all complete commands in the buffer first
            while (true)
            {
                const std::size_t newline_pos = line.find('\n');
                if (newline_pos == std::string::npos)
                {
                    break;
                }

                const std::string command_line = line.substr(0, newline_pos);
                line.erase(0, newline_pos + 1);

                const Command command = parser.parse(command_line);
                const std::string response = handler.handle(command);
                send_response(client_fd, response);

                if (command.type == CommandType::EXIT)
                {
                    close(client_fd);
                    return 0;
                }
            }

            // Now try to read more data
            const ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);
            if (bytes_read <= 0)
            {
                break;
            }

            line.append(buffer, bytes_read);
        }

        close(client_fd);
    }
}
