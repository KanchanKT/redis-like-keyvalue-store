#include "key_value_store.h"

bool KeyValueStore::set(const std::string& key,
                        const std::string& value)
{
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
    database_.clear();
}