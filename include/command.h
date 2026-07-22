#pragma once

#include <string>

#include "types.h"

struct Command
{
    CommandType type = CommandType::INVALID;
    std::string key;
    std::string value;
};