#include "key_value_store.h"

#include <cassert>
#include <filesystem>

int main()
{
    KeyValueStore store;

    assert(store.set("name", "Kanchan"));

    const auto value = store.get("name");
    assert(value.has_value());
    assert(value.value() == "Kanchan");

    assert(store.exists("name"));
    assert(store.remove("name"));
    assert(!store.exists("name"));

    const auto missing = store.get("name");
    assert(!missing.has_value());

    const std::filesystem::path wal_path =
        std::filesystem::temp_directory_path() /
        "redis_like_store_persistence_test.log";
    std::filesystem::remove(wal_path);

    {
        KeyValueStore persistent_store(wal_path.string());
        assert(persistent_store.set("name", "Kanchan"));
    }

    {
        KeyValueStore recovered_store(wal_path.string());
        const auto recovered_value = recovered_store.get("name");
        assert(recovered_value.has_value());
        assert(recovered_value.value() == "Kanchan");
    }

    std::filesystem::remove(wal_path);

    return 0;
}