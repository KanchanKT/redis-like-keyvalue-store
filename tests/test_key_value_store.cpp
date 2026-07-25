#include "key_value_store.h"

#include <atomic>
#include <cassert>
#include <filesystem>
#include <thread>
#include <unordered_map>
#include <vector>

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
        const std::filesystem::path concurrent_wal_path =
            std::filesystem::temp_directory_path() /
            "redis_like_store_concurrent_wal.log";
        std::filesystem::remove(concurrent_wal_path);

        WriteAheadLog concurrent_wal(concurrent_wal_path.string());

        constexpr int kThreads = 4;
        constexpr int kOpsPerThread = 50;
        std::atomic<bool> start(false);
        std::vector<std::thread> threads;
        threads.reserve(kThreads);

        for (int thread_idx = 0; thread_idx < kThreads; ++thread_idx)
        {
            threads.emplace_back([&concurrent_wal, &start, thread_idx]()
            {
                while (!start.load(std::memory_order_relaxed))
                {
                    std::this_thread::yield();
                }

                for (int op_idx = 0; op_idx < kOpsPerThread; ++op_idx)
                {
                    const std::string key =
                        "k" + std::to_string(thread_idx) + "_" + std::to_string(op_idx);
                    const std::string value =
                        "value-" + std::to_string(thread_idx) + "-" + std::to_string(op_idx);
                    assert(concurrent_wal.append_set(key, value));
                }
            });
        }

        start.store(true, std::memory_order_release);
        for (auto& worker : threads)
        {
            worker.join();
        }

        std::unordered_map<std::string, std::string> replayed_values;
        concurrent_wal.replay(
            [&replayed_values](const std::string& key, const std::string& value)
            {
                replayed_values[key] = value;
            },
            [](const std::string&) {},
            []() {});

        assert(replayed_values.size() == static_cast<size_t>(kThreads * kOpsPerThread));
        std::filesystem::remove(concurrent_wal_path);
    }

    {
        KeyValueStore persistent_store(wal_path.string());
        assert(persistent_store.set("name", "Kanchan"));
        assert(persistent_store.set("greeting", "hello world"));
    }

    {
        KeyValueStore recovered_store(wal_path.string());
        const auto recovered_value = recovered_store.get("name");
        assert(recovered_value.has_value());
        assert(recovered_value.value() == "Kanchan");

        const auto recovered_greeting = recovered_store.get("greeting");
        assert(recovered_greeting.has_value());
        assert(recovered_greeting.value() == "hello world");
    }

    std::filesystem::remove(wal_path);

    return 0;
}