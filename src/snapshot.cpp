#include "snapshot.h"

#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

Snapshot::Snapshot(std::string file_path)
    : file_path_(std::move(file_path))
{
}

bool Snapshot::save(const std::unordered_map<std::string, std::string>& data)
{
    const int fd = ::open(file_path_.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0)
    {
        return false;
    }

    const std::uint32_t num_entries = static_cast<std::uint32_t>(data.size());
    const std::uint32_t network_count = htonl(num_entries);

    if (::write(fd, &network_count, sizeof(network_count)) < 0)
    {
        ::close(fd);
        return false;
    }

    for (const auto& [key, value] : data)
    {
        const std::uint32_t key_len = static_cast<std::uint32_t>(key.size());
        const std::uint32_t key_len_net = htonl(key_len);

        if (::write(fd, &key_len_net, sizeof(key_len_net)) < 0)
        {
            ::close(fd);
            return false;
        }

        if (key_len > 0 && ::write(fd, key.data(), key_len) < 0)
        {
            ::close(fd);
            return false;
        }

        const std::uint32_t value_len = static_cast<std::uint32_t>(value.size());
        const std::uint32_t value_len_net = htonl(value_len);

        if (::write(fd, &value_len_net, sizeof(value_len_net)) < 0)
        {
            ::close(fd);
            return false;
        }

        if (value_len > 0 && ::write(fd, value.data(), value_len) < 0)
        {
            ::close(fd);
            return false;
        }
    }

    return ::close(fd) == 0;
}

bool Snapshot::load(std::unordered_map<std::string, std::string>& data)
{
    const int fd = ::open(file_path_.c_str(), O_RDONLY);
    if (fd < 0)
    {
        return false;
    }

    std::uint32_t network_count = 0;
    if (::read(fd, &network_count, sizeof(network_count)) < static_cast<ssize_t>(sizeof(network_count)))
    {
        ::close(fd);
        return false;
    }

    const std::uint32_t num_entries = ntohl(network_count);

    for (std::uint32_t i = 0; i < num_entries; ++i)
    {
        std::uint32_t key_len_net = 0;
        if (::read(fd, &key_len_net, sizeof(key_len_net)) < static_cast<ssize_t>(sizeof(key_len_net)))
        {
            ::close(fd);
            return false;
        }

        const std::uint32_t key_len = ntohl(key_len_net);
        std::string key;
        key.resize(key_len);

        if (key_len > 0 && ::read(fd, key.data(), key_len) < static_cast<ssize_t>(key_len))
        {
            ::close(fd);
            return false;
        }

        std::uint32_t value_len_net = 0;
        if (::read(fd, &value_len_net, sizeof(value_len_net)) < static_cast<ssize_t>(sizeof(value_len_net)))
        {
            ::close(fd);
            return false;
        }

        const std::uint32_t value_len = ntohl(value_len_net);
        std::string value;
        value.resize(value_len);

        if (value_len > 0 && ::read(fd, value.data(), value_len) < static_cast<ssize_t>(value_len))
        {
            ::close(fd);
            return false;
        }

        data[key] = value;
    }

    ::close(fd);
    return true;
}

bool Snapshot::exists() const
{
    return ::access(file_path_.c_str(), F_OK) == 0;
}
