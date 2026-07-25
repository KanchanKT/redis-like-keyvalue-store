#include "replication.h"

#include "cluster_state.h"

#include <iostream>
#include <mutex>

Replication::Replication(std::shared_ptr<ClusterState> cluster_state)
    : cluster_state_(std::move(cluster_state))
    , running_(false)
    , replicated_count_(0)
    , failed_replication_count_(0)
{
}

Replication::~Replication()
{
    stop();
}

void Replication::send_heartbeats()
{
    if (!cluster_state_->is_leader())
    {
        return;
    }

    auto followers = cluster_state_->get_followers();
    for (auto& follower : followers)
    {
        send_heartbeat_to(follower->id);
    }
}

void Replication::replicate_command(const std::string& operation, const std::string& key, const std::string& value)
{
    if (!cluster_state_->is_leader())
    {
        return;
    }

    auto followers = cluster_state_->get_followers();
    for (auto& follower : followers)
    {
        if (send_replication_to(follower->id, operation, key, value))
        {
            std::unique_lock lock(mutex_);
            replicated_count_++;
        }
        else
        {
            std::unique_lock lock(mutex_);
            failed_replication_count_++;
        }
    }
}

void Replication::start()
{
    std::unique_lock lock(mutex_);
    if (running_)
    {
        return;
    }

    running_ = true;
    heartbeat_thread_ = std::thread([this]() { heartbeat_loop(); });
}

void Replication::stop()
{
    {
        std::unique_lock lock(mutex_);
        running_ = false;
    }

    if (heartbeat_thread_.joinable())
    {
        heartbeat_thread_.join();
    }
}

Replication::ReplicationStats Replication::get_stats()
{
    std::shared_lock lock(mutex_);
    return ReplicationStats{
        replicated_count_,
        failed_replication_count_,
        std::chrono::milliseconds(0)
    };
}

bool Replication::send_heartbeat_to(const std::string& node_id)
{
    // In a full implementation, this would establish a socket connection
    // and send a heartbeat message. For now, just update the node state.
    cluster_state_->update_node_heartbeat(node_id);
    return true;
}

bool Replication::send_replication_to(const std::string& node_id, const std::string& operation,
                                      const std::string& key, const std::string& value)
{
    // In a full implementation, this would send replication data to the follower.
    // For now, just mark as acknowledged.
    auto node = cluster_state_->get_node(node_id);
    if (!node)
    {
        return false;
    }

    node->match_index++;
    node->next_index++;
    return true;
}

void Replication::heartbeat_loop()
{
    while (running_)
    {
        // Send heartbeats every 100ms when leader
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (cluster_state_->is_leader())
        {
            send_heartbeats();
        }
    }
}
