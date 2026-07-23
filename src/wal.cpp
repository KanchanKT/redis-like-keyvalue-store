#include "wal.h"

#include <fstream>

#include "command_parser.h"

WriteAheadLog::WriteAheadLog(std::string file_path)
    : file_path_(std::move(file_path))
{
}

bool WriteAheadLog::append_set(const std::string& key,
                               const std::string& value)
{
    std::ofstream wal_stream(file_path_, std::ios::app);
    if (!wal_stream.is_open())
    {
        return false;
    }

    wal_stream << "SET " << key << ' ' << value << '\n';
    wal_stream.flush();
    return true;
}

bool WriteAheadLog::append_delete(const std::string& key)
{
    std::ofstream wal_stream(file_path_, std::ios::app);
    if (!wal_stream.is_open())
    {
        return false;
    }

    wal_stream << "DELETE " << key << '\n';
    wal_stream.flush();
    return true;
}

bool WriteAheadLog::append_clear()
{
    std::ofstream wal_stream(file_path_, std::ios::app);
    if (!wal_stream.is_open())
    {
        return false;
    }

    wal_stream << "CLEAR\n";
    wal_stream.flush();
    return true;
}

bool WriteAheadLog::replay(
    const std::function<void(const std::string&, const std::string&)>& on_set,
    const std::function<void(const std::string&)>& on_delete,
    const std::function<void()>& on_clear)
{
    std::ifstream wal_stream(file_path_);
    if (!wal_stream.is_open())
    {
        return true;
    }

    CommandParser parser;
    std::string line;

    while (std::getline(wal_stream, line))
    {
        if (line.empty())
        {
            continue;
        }

        const Command command = parser.parse(line);
        switch (command.type)
        {
            case CommandType::SET:
                on_set(command.key, command.value);
                break;
            case CommandType::DELETE:
                on_delete(command.key);
                break;
            case CommandType::CLEAR:
                on_clear();
                break;
            default:
                break;
        }
    }

    return true;
}
