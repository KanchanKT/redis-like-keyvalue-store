#include "wal.h"

#include <arpa/inet.h>
#include <cerrno>
#include <filesystem>
#include <fcntl.h>
#include <unistd.h>

WriteAheadLog::WriteAheadLog(std::string file_path)
    : fd_(-1)
    , file_path_(std::move(file_path))
{
    if (!open_file())
    {
        return;
    }
}

WriteAheadLog::~WriteAheadLog()
{
    if (fd_ >= 0)
    {
        ::close(fd_);
    }
}

bool WriteAheadLog::append_set(const std::string& key,
                               const std::string& value)
{
    return append_record(RecordType::Set, key, value);
}

bool WriteAheadLog::append_delete(const std::string& key)
{
    return append_record(RecordType::Delete, key, {});
}

bool WriteAheadLog::append_clear()
{
    return append_record(RecordType::Clear, {}, {});
}

bool WriteAheadLog::replay(
    const std::function<void(const std::string&, const std::string&)>& on_set,
    const std::function<void(const std::string&)>& on_delete,
    const std::function<void()>& on_clear)
{
    const int replay_fd = ::open(file_path_.c_str(), O_RDONLY);
    if (replay_fd < 0)
    {
        return errno == ENOENT;
    }

    while (true)
    {
        std::uint8_t type_byte = 0;
        if (!read_exact(replay_fd, reinterpret_cast<char*>(&type_byte), 1))
        {
            break;
        }

        const auto record_type = static_cast<RecordType>(type_byte);
        std::uint32_t key_length = 0;
        if (!read_u32(replay_fd, key_length))
        {
            ::close(replay_fd);
            return false;
        }

        std::string key;
        key.resize(key_length);
        if (key_length > 0 && !read_exact(replay_fd, key.data(), key_length))
        {
            ::close(replay_fd);
            return false;
        }

        std::uint32_t value_length = 0;
        if (!read_u32(replay_fd, value_length))
        {
            ::close(replay_fd);
            return false;
        }

        std::string value;
        value.resize(value_length);
        if (value_length > 0 && !read_exact(replay_fd, value.data(), value_length))
        {
            ::close(replay_fd);
            return false;
        }

        switch (record_type)
        {
            case RecordType::Set:
                on_set(key, value);
                break;
            case RecordType::Delete:
                on_delete(key);
                break;
            case RecordType::Clear:
                on_clear();
                break;
            default:
                break;
        }
    }

    ::close(replay_fd);
    return true;
}

bool WriteAheadLog::open_file()
{
    const std::filesystem::path wal_path(file_path_);
    if (!wal_path.parent_path().empty())
    {
        std::error_code ec;
        std::filesystem::create_directories(wal_path.parent_path(), ec);
        if (ec)
        {
            return false;
        }
    }

    fd_ = ::open(file_path_.c_str(), O_CREAT | O_APPEND | O_WRONLY, 0644);
    return fd_ >= 0;
}

bool WriteAheadLog::append_record(RecordType type,
                                   const std::string& key,
                                   const std::string& value)
{
    if (fd_ < 0 && !open_file())
    {
        return false;
    }

    const std::uint8_t record_type = static_cast<std::uint8_t>(type);
    if (!write_exact(fd_, reinterpret_cast<const char*>(&record_type), 1))
    {
        return false;
    }

    const std::uint32_t key_length = static_cast<std::uint32_t>(key.size());
    if (!write_u32(fd_, key_length))
    {
        return false;
    }

    if (key_length > 0 && !write_exact(fd_, key.data(), key_length))
    {
        return false;
    }

    const std::uint32_t value_length = static_cast<std::uint32_t>(value.size());
    if (!write_u32(fd_, value_length))
    {
        return false;
    }

    if (value_length > 0 && !write_exact(fd_, value.data(), value_length))
    {
        return false;
    }

    return flush_and_sync();
}

bool WriteAheadLog::flush_and_sync()
{
    if (fd_ < 0)
    {
        return false;
    }

    return ::fsync(fd_) == 0;
}

bool WriteAheadLog::write_exact(int fd, const char* data, std::size_t size)
{
    std::size_t written = 0;
    while (written < size)
    {
        const ssize_t result = ::write(fd, data + written, size - written);
        if (result < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            return false;
        }

        written += static_cast<std::size_t>(result);
    }

    return true;
}

bool WriteAheadLog::read_exact(int fd, char* data, std::size_t size)
{
    std::size_t read_bytes = 0;
    while (read_bytes < size)
    {
        const ssize_t result = ::read(fd, data + read_bytes, size - read_bytes);
        if (result < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            return false;
        }

        if (result == 0)
        {
            return false;
        }

        read_bytes += static_cast<std::size_t>(result);
    }

    return true;
}

bool WriteAheadLog::write_u32(int fd, std::uint32_t value)
{
    const std::uint32_t network_value = htonl(value);
    return write_exact(fd, reinterpret_cast<const char*>(&network_value), sizeof(network_value));
}

bool WriteAheadLog::read_u32(int fd, std::uint32_t& value)
{
    std::uint32_t network_value = 0;
    if (!read_exact(fd, reinterpret_cast<char*>(&network_value), sizeof(network_value)))
    {
        return false;
    }

    value = ntohl(network_value);
    return true;
}
