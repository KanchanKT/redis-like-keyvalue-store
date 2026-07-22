#include "key_value_store.h"

#include <cassert>

int main()
{
    KeyValueStore store;

    assert(store.set("name", "Alice"));

    const auto value = store.get("name");
    assert(value.has_value());
    assert(value.value() == "Alice");

    assert(store.exists("name"));
    assert(store.remove("name"));
    assert(!store.exists("name"));

    const auto missing = store.get("name");
    assert(!missing.has_value());

    return 0;
}