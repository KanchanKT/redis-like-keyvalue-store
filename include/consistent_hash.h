#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

class ConsistentHash
{
public:
    explicit ConsistentHash(size_t virtual_nodes = 160);

    // Add a node to the hash ring
    void add_node(const std::string& node_id);

    // Remove a node from the hash ring
    void remove_node(const std::string& node_id);

    // Get the node responsible for a key
    std::string get_node(const std::string& key) const;

    // Get all nodes in the ring
    std::vector<std::string> get_all_nodes() const;

    // Get replicas (next N nodes in ring) for a key
    std::vector<std::string> get_replicas(const std::string& key, size_t count) const;

private:
    struct RingEntry
    {
        uint32_t hash;
        std::string node_id;
    };

    uint32_t hash_key(const std::string& key) const;

    std::map<uint32_t, std::string> ring_;
    size_t virtual_nodes_;
};
