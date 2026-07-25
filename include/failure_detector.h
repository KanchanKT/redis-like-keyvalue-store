#pragma once

#include <chrono>
#include <memory>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

class ClusterState;

class FailureDetector
{
public:
    explicit FailureDetector(std::shared_ptr<ClusterState> cluster_state,
                             std::chrono::milliseconds heartbeat_timeout = std::chrono::milliseconds(3000));
    ~FailureDetector();

    // Start failure detection background thread
    void start();

    // Stop failure detection
    void stop();

    // Get nodes that are currently suspected as failed
    std::vector<std::string> get_suspected_failed();

    // Get nodes that are confirmed failed
    std::vector<std::string> get_confirmed_failed();

    // Manually mark a node as failed (e.g., connection refused)
    void mark_node_failed(const std::string& node_id);

    // Manually mark a node as recovered
    void mark_node_recovered(const std::string& node_id);

private:
    std::shared_ptr<ClusterState> cluster_state_;
    std::chrono::milliseconds heartbeat_timeout_;
    bool running_;
    std::thread detection_thread_;
    mutable std::shared_mutex mutex_;

    std::vector<std::string> suspected_failed_;
    std::vector<std::string> confirmed_failed_;

    // Background failure detection loop
    void detection_loop();

    // Check node health and update failure status
    void check_node_health(const std::string& node_id);

    // Trigger failover if leader is detected as failed
    void trigger_failover();
};
