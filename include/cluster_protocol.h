#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Cluster protocol message types
enum class ClusterMessageType
{
    HEARTBEAT = 1,           // Leader heartbeat
    APPEND_ENTRIES = 2,      // Replicate log entries
    REQUEST_VOTE = 3,        // Leader election
    VOTE = 4,                // Vote response
    REPLICATION_RESPONSE = 5 // Follower replication response
};

// Cluster protocol message format
struct ClusterMessage
{
    ClusterMessageType type;
    uint64_t term;           // Current term
    std::string from;        // Sender node ID
    std::string to;          // Recipient node ID
    uint64_t prev_index;     // Previous log index (for APPEND_ENTRIES)
    uint64_t prev_term;      // Previous log term
    uint64_t leader_commit;  // Leader's committed index
    uint64_t match_index;    // For replication acknowledgment
    std::vector<uint8_t> entries;  // Log entries or command data
};

// Heartbeat message - minimal protocol for liveness
struct HeartbeatMessage
{
    uint64_t term;           // Current term
    std::string from;        // Sender node ID
    uint64_t leader_commit;  // Leader's latest commit
};

// Replication request - send changes to follower
struct ReplicationRequest
{
    uint64_t term;
    std::string leader_id;
    uint64_t prev_index;
    uint64_t leader_commit;
    std::string operation;   // SET, DELETE, CLEAR
    std::string key;
    std::string value;
};

// Replication response - follower acknowledges
struct ReplicationResponse
{
    uint64_t term;
    std::string follower_id;
    uint64_t match_index;
    bool success;
};
