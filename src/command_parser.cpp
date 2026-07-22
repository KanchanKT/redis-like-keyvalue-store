#include "command_parser.h"

#include <sstream>

Command CommandParser::parse(const std::string& input) const
{
    std::stringstream ss(input);

    std::string operation;
    Command command;

    ss >> operation;

    if (operation == "SET")
    {
        command.type = CommandType::SET;
        ss >> command.key;
        ss >> command.value;
    }
    else if (operation == "GET")
    {
        command.type = CommandType::GET;
        ss >> command.key;
    }
    else if (operation == "DELETE")
    {
        command.type = CommandType::DELETE;
        ss >> command.key;
    }
    else if (operation == "EXISTS")
    {
        command.type = CommandType::EXISTS;
        ss >> command.key;
    }
    else if (operation == "SIZE")
    {
        command.type = CommandType::SIZE;
    }
    else if (operation == "CLEAR")
    {
        command.type = CommandType::CLEAR;
    }
    else if (operation == "HELP")
    {
        command.type = CommandType::HELP;
    }
    else if (operation == "EXIT")
    {
        command.type = CommandType::EXIT;
    }
    else
    {
        command.type = CommandType::INVALID;
    }

    return command;
}