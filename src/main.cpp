#include <iostream>
#include <optional>
#include <string>

#include "command_parser.h"
#include "key_value_store.h"

int main()
{
    CommandParser parser;
    KeyValueStore store("wal.log");

    std::string line;

    while (true)
    {
        std::cout << "> ";

        std::getline(std::cin, line);

        Command command = parser.parse(line);

        switch (command.type)
        {
            case CommandType::SET:
                store.set(command.key, command.value);
                std::cout << "SET command\n";
                std::cout << "Key   : " << command.key << '\n';
                std::cout << "Value : " << command.value << '\n';
                break;

            case CommandType::GET:
            {
                std::cout << "GET command\n";
                std::cout << "Key : " << command.key << '\n';
                const std::optional<std::string> value = store.get(command.key);
                if (value.has_value())
                {
                    std::cout << "Value : " << value.value() << '\n';
                }
                else
                {
                    std::cout << "Value : <not found>\n";
                }
                break;
            }

            case CommandType::DELETE:
                if (store.remove(command.key))
                {
                    std::cout << "DELETE command\n";
                    std::cout << "Key : " << command.key << '\n';
                    std::cout << "Status : deleted\n";
                }
                else
                {
                    std::cout << "DELETE command\n";
                    std::cout << "Key : " << command.key << '\n';
                    std::cout << "Status : not found\n";
                }
                break;

            case CommandType::EXISTS:
                std::cout << "EXISTS command\n";
                std::cout << "Key : " << command.key << '\n';
                std::cout << "Status : " << (store.exists(command.key) ? "true" : "false") << '\n';
                break;

            case CommandType::SIZE:
                std::cout << "SIZE command\n";
                std::cout << "Count : " << store.size() << '\n';
                break;

            case CommandType::CLEAR:
                store.clear();
                std::cout << "CLEAR command\n";
                break;

            case CommandType::HELP:
                std::cout << "Supported commands: SET, GET, DELETE, EXISTS, SIZE, CLEAR, HELP, EXIT\n";
                break;

            case CommandType::EXIT:
                return 0;

            default:
                std::cout << "Invalid command\n";
        }
    }
}