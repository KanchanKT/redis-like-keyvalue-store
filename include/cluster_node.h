#pragma once

#include <cstdint>
#include <chrono>
#include <string>

enum class NodeRole
{
    LEADER,
    FOLLOWER,
    CANDIDATE
};

struct ClusterNode
{
    std::string id;           // Unique node ID (host:port)
    std::string host;         // IP address
    int port;                 // Port number
    NodeRole role;            // LEADER, FOLLOWER, or CANDIDATE
    uint64_t term;            // Current term (for leader election)
    bool is_alive;            // Health status
    std::chrono::steady_clock::time_point last_heartbeat;  // Last heartbeat received
    uint64_t applied_index;   // Last applied log index
    uint64_t match_index;     // Last replicated index
    uint64_t next_index;      // Next index to replicate

    ClusterNode() = default;
    explicit ClusterNode(std::string node_id, std::string h = "", int p = 0);

    // Check if the node is healthy (heartbeat within timeout)
    bool is_healthy(std::chrono::milliseconds heartbeat_timeout) const;

    // Update the heartbeat timestamp
    void update_heartbeat();
};
