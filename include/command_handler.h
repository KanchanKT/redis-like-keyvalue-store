#pragma once

#include <string>

#include "command.h"
#include "key_value_store.h"

class CommandHandler
{
public:
    explicit CommandHandler(KeyValueStore& store);

    std::string handle(const Command& command);

private:
    KeyValueStore& store_;
};
