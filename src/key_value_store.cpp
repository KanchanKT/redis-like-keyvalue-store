#include "key_value_store.h"

KeyValueStore::KeyValueStore(std::string wal_path)
    : wal_(std::move(wal_path))
{
    wal_.replay(
        [this](const std::string& key, const std::string& value)
        {
            database_[key] = value;
        },
        [this](const std::string& key)
        {
            database_.erase(key);
        },
        [this]()
        {
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

    database_[key] = value;
    return true;
}

std::optional<std::string> KeyValueStore::get(
    const std::string& key) const
{
    auto it = database_.find(key);

    if (it == database_.end())
    {
        return std::nullopt;
    }

    return it->second;
}

bool KeyValueStore::remove(const std::string& key)
{
    if (!exists(key))
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
    return database_.find(key) != database_.end();
}

size_t KeyValueStore::size() const
{
    return database_.size();
}

void KeyValueStore::clear()
{
    if (!wal_.append_clear())
    {
        return;
    }

    database_.clear();
}