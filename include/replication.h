#pragma once

#include <chrono>
#include <memory>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

class ClusterState;

class Replication
{
public:
    explicit Replication(std::shared_ptr<ClusterState> cluster_state);
    ~Replication();

    // Send heartbeat to all followers
    void send_heartbeats();

    // Replicate a command to all followers (leader only)
    void replicate_command(const std::string& operation, const std::string& key, const std::string& value);

    // Start replication background thread
    void start();

    // Stop replication thread
    void stop();

    // Get replication stats
    struct ReplicationStats
    {
        uint64_t replicated_count;
        uint64_t failed_replication_count;
        std::chrono::milliseconds avg_replication_latency;
    };
    ReplicationStats get_stats();

private:
    std::shared_ptr<ClusterState> cluster_state_;
    bool running_;
    std::thread heartbeat_thread_;
    mutable std::shared_mutex mutex_;

    // Send heartbeat to a specific node
    bool send_heartbeat_to(const std::string& node_id);

    // Send replication request to a specific node
    bool send_replication_to(const std::string& node_id, const std::string& operation,
                             const std::string& key, const std::string& value);

    // Background heartbeat loop
    void heartbeat_loop();

    // Stats tracking
    uint64_t replicated_count_;
    uint64_t failed_replication_count_;
};
