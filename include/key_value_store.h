#pragma once

#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "snapshot.h"
#include "wal.h"

class KeyValueStore
{
public:
    explicit KeyValueStore(std::string wal_path = "wal.log",
                           std::string snapshot_path = "snapshot.bin");

    bool set(const std::string& key, const std::string& value);

    std::optional<std::string> get(const std::string& key) const;

    bool remove(const std::string& key);

    bool exists(const std::string& key) const;

    size_t size() const;

    void clear();

    bool create_snapshot();

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::string> database_;
    WriteAheadLog wal_;
    Snapshot snapshot_;
};