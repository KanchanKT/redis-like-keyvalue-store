#pragma once

#include <string>

#include "command.h"

class CommandParser
{
public:
    Command parse(const std::string& input) const;
};