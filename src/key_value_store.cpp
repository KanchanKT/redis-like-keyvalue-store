#include "key_value_store.h"

#include <mutex>

KeyValueStore::KeyValueStore(std::string wal_path, std::string snapshot_path)
    : wal_(std::move(wal_path))
    , snapshot_(std::move(snapshot_path))
{
    // First, try to load from snapshot
    if (snapshot_.exists())
    {
        snapshot_.load(database_);
    }

    // Then replay WAL to get all changes since the snapshot
    wal_.replay(
        [this](const std::string& key, const std::string& value)
        {
            std::unique_lock lock(mutex_);
            database_[key] = value;
        },
        [this](const std::string& key)
        {
            std::unique_lock lock(mutex_);
            database_.erase(key);
        },
        [this]()
        {
            std::unique_lock lock(mutex_);
            database_.clear();
        });
}

bool KeyValueStore::set(const std::string& key,
                        const std::string& value)
{
    if (!wal_.append_set(key, value))
    {
        return false;
    }

    std::unique_lock lock(mutex_);
    database_[key] = value;
    return true;
}

std::optional<std::string> KeyValueStore::get(
    const std::string& key) const
{
    std::shared_lock lock(mutex_);
    auto it = database_.find(key);

    if (it == database_.end())
    {
        return std::nullopt;
    }

    return it->second;
}

bool KeyValueStore::remove(const std::string& key)
{
    std::unique_lock lock(mutex_);
    if (database_.find(key) == database_.end())
    {
        return false;
    }

    if (!wal_.append_delete(key))
    {
        return false;
    }

    return database_.erase(key) > 0;
}

bool KeyValueStore::exists(const std::string& key) const
{
    std::shared_lock lock(mutex_);
    return database_.find(key) != database_.end();
}

size_t KeyValueStore::size() const
{
    std::shared_lock lock(mutex_);
    return database_.size();
}

void KeyValueStore::clear()
{
    if (!wal_.append_clear())
    {
        return;
    }

    std::unique_lock lock(mutex_);
    database_.clear();
}

bool KeyValueStore::create_snapshot()
{
    std::shared_lock lock(mutex_);

    if (!snapshot_.save(database_))
    {
        return false;
    }

    // After snapshot, we can safely discard old WAL entries
    lock.unlock();
    return wal_.truncate();
}
