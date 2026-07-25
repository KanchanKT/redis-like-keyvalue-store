#include "command_handler.h"

#include <optional>
#include <string>

CommandHandler::CommandHandler(KeyValueStore& store)
    : store_(store)
{
}

std::string CommandHandler::handle(const Command& command)
{
    switch (command.type)
    {
        case CommandType::SET:
            store_.set(command.key, command.value);
            return "OK\n";

        case CommandType::GET:
        {
            const std::optional<std::string> value = store_.get(command.key);
            if (!value.has_value())
            {
                return "<not found>\n";
            }
            return value.value() + "\n";
        }

        case CommandType::DELETE:
            return store_.remove(command.key) ? "DELETED\n" : "NOT_FOUND\n";

        case CommandType::EXISTS:
            return store_.exists(command.key) ? "true\n" : "false\n";

        case CommandType::SIZE:
            return std::to_string(store_.size()) + "\n";

        case CommandType::CLEAR:
            store_.clear();
            return "CLEARED\n";

        case CommandType::HELP:
            return "Supported commands: SET, GET, DELETE, EXISTS, SIZE, CLEAR, HELP, EXIT\n";

        case CommandType::EXIT:
            return "BYE\n";

        default:
            return "INVALID\n";
    }
}
