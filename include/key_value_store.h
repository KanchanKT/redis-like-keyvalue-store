#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "wal.h"

class KeyValueStore
{
public:
    explicit KeyValueStore(std::string wal_path = "wal.log");

    bool set(const std::string& key, const std::string& value);

    std::optional<std::string> get(const std::string& key) const;

    bool remove(const std::string& key);

    bool exists(const std::string& key) const;

    size_t size() const;

    void clear();

private:
    std::unordered_map<std::string, std::string> database_;
    WriteAheadLog wal_;
};