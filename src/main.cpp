#include <iostream>
#include <string>

#include "command_parser.h"

int main()
{
    CommandParser parser;

    std::string line;

    while (true)
    {
        std::cout << "> ";

        std::getline(std::cin, line);

        Command command = parser.parse(line);

        switch (command.type)
        {
            case CommandType::SET:
                std::cout << "SET command\n";
                std::cout << "Key   : " << command.key << '\n';
                std::cout << "Value : " << command.value << '\n';
                break;

            case CommandType::GET:
                std::cout << "GET command\n";
                std::cout << "Key : " << command.key << '\n';
                break;

            case CommandType::EXIT:
                return 0;

            default:
                std::cout << "Invalid command\n";
        }
    }
}