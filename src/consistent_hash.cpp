#include "consistent_hash.h"

#include <algorithm>
#include <cstring>

// FNV-1a hash function for consistent hashing
static uint32_t fnv1a_hash(const std::string& key)
{
    uint32_t hash = 2166136261u;
    const uint32_t fnv_prime = 16777619u;

    for (unsigned char c : key)
    {
        hash ^= c;
        hash *= fnv_prime;
    }

    return hash;
}

ConsistentHash::ConsistentHash(size_t virtual_nodes)
    : virtual_nodes_(virtual_nodes)
{
}

void ConsistentHash::add_node(const std::string& node_id)
{
    for (size_t i = 0; i < virtual_nodes_; ++i)
    {
        std::string virtual_key = node_id + ":" + std::to_string(i);
        uint32_t hash = hash_key(virtual_key);
        ring_[hash] = node_id;
    }
}

void ConsistentHash::remove_node(const std::string& node_id)
{
    // Remove all virtual nodes for this node_id
    std::vector<uint32_t> to_remove;
    for (const auto& [hash, node] : ring_)
    {
        if (node == node_id)
        {
            to_remove.push_back(hash);
        }
    }

    for (uint32_t hash : to_remove)
    {
        ring_.erase(hash);
    }
}

std::string ConsistentHash::get_node(const std::string& key) const
{
    if (ring_.empty())
    {
        return "";
    }

    uint32_t key_hash = hash_key(key);

    // Find the first node with hash >= key_hash
    auto it = ring_.lower_bound(key_hash);

    // If not found, wrap around to the first node in the ring
    if (it == ring_.end())
    {
        it = ring_.begin();
    }

    return it->second;
}

std::vector<std::string> ConsistentHash::get_all_nodes() const
{
    std::vector<std::string> nodes;
    for (const auto& [hash, node_id] : ring_)
    {
        // Only add unique node IDs
        if (std::find(nodes.begin(), nodes.end(), node_id) == nodes.end())
        {
            nodes.push_back(node_id);
        }
    }
    return nodes;
}

std::vector<std::string> ConsistentHash::get_replicas(const std::string& key, size_t count) const
{
    std::vector<std::string> replicas;

    if (ring_.empty())
    {
        return replicas;
    }

    uint32_t key_hash = hash_key(key);
    auto it = ring_.lower_bound(key_hash);

    if (it == ring_.end())
    {
        it = ring_.begin();
    }

    // Get the next 'count' unique nodes
    std::string last_node;
    while (replicas.size() < count && replicas.size() < ring_.size())
    {
        if (it->second != last_node)
        {
            replicas.push_back(it->second);
            last_node = it->second;
        }

        ++it;
        if (it == ring_.end())
        {
            it = ring_.begin();
        }
    }

    return replicas;
}

uint32_t ConsistentHash::hash_key(const std::string& key) const
{
    return fnv1a_hash(key);
}
