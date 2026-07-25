#pragma once

#include "cluster_node.h"

#include <chrono>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

class ClusterState
{
public:
    explicit ClusterState(std::string self_id, int self_port);

    // Add a node to the cluster
    void add_node(const std::string& node_id, const std::string& host, int port);

    // Remove a node from the cluster
    void remove_node(const std::string& node_id);

    // Get a node by ID
    std::shared_ptr<ClusterNode> get_node(const std::string& node_id);

    // Get all nodes
    std::vector<std::shared_ptr<ClusterNode>> get_all_nodes();

    // Get all follower nodes (for replication)
    std::vector<std::shared_ptr<ClusterNode>> get_followers();

    // Current term (for leader election)
    uint64_t get_term() const;
    void set_term(uint64_t term);

    // Leadership
    bool is_leader() const;
    void become_leader();
    void become_follower();

    std::string get_self_id() const;

    // Node health monitoring
    void update_node_heartbeat(const std::string& node_id);
    std::vector<std::string> get_unhealthy_nodes(std::chrono::milliseconds heartbeat_timeout);

private:
    mutable std::shared_mutex mutex_;
    std::string self_id_;
    int self_port_;
    uint64_t current_term_;
    std::string leader_id_;
    std::map<std::string, std::shared_ptr<ClusterNode>> nodes_;
};
