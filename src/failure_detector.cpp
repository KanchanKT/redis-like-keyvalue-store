#include "failure_detector.h"

#include "cluster_state.h"

#include <algorithm>
#include <iostream>
#include <mutex>

FailureDetector::FailureDetector(std::shared_ptr<ClusterState> cluster_state,
                                 std::chrono::milliseconds heartbeat_timeout)
    : cluster_state_(std::move(cluster_state))
    , heartbeat_timeout_(heartbeat_timeout)
    , running_(false)
{
}

FailureDetector::~FailureDetector()
{
    stop();
}

void FailureDetector::start()
{
    std::unique_lock lock(mutex_);
    if (running_)
    {
        return;
    }

    running_ = true;
    detection_thread_ = std::thread([this]() { detection_loop(); });
}

void FailureDetector::stop()
{
    {
        std::unique_lock lock(mutex_);
        running_ = false;
    }

    if (detection_thread_.joinable())
    {
        detection_thread_.join();
    }
}

std::vector<std::string> FailureDetector::get_suspected_failed()
{
    std::shared_lock lock(mutex_);
    return suspected_failed_;
}

std::vector<std::string> FailureDetector::get_confirmed_failed()
{
    std::shared_lock lock(mutex_);
    return confirmed_failed_;
}

void FailureDetector::mark_node_failed(const std::string& node_id)
{
    std::unique_lock lock(mutex_);
    if (std::find(confirmed_failed_.begin(), confirmed_failed_.end(), node_id) == confirmed_failed_.end())
    {
        confirmed_failed_.push_back(node_id);
    }

    auto node = cluster_state_->get_node(node_id);
    if (node)
    {
        node->is_alive = false;
    }
}

void FailureDetector::mark_node_recovered(const std::string& node_id)
{
    std::unique_lock lock(mutex_);
    confirmed_failed_.erase(
        std::remove(confirmed_failed_.begin(), confirmed_failed_.end(), node_id),
        confirmed_failed_.end());
    suspected_failed_.erase(
        std::remove(suspected_failed_.begin(), suspected_failed_.end(), node_id),
        suspected_failed_.end());

    auto node = cluster_state_->get_node(node_id);
    if (node)
    {
        node->is_alive = true;
        node->update_heartbeat();
    }
}

void FailureDetector::detection_loop()
{
    while (running_)
    {
        // Check health every 500ms
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        auto nodes = cluster_state_->get_all_nodes();
        for (const auto& node : nodes)
        {
            if (node->id != cluster_state_->get_self_id())
            {
                check_node_health(node->id);
            }
        }

        // Trigger failover if leader is failed
        trigger_failover();
    }
}

void FailureDetector::check_node_health(const std::string& node_id)
{
    auto node = cluster_state_->get_node(node_id);
    if (!node)
    {
        return;
    }

    if (!node->is_healthy(heartbeat_timeout_))
    {
        std::unique_lock lock(mutex_);

        // Move from suspected to confirmed if already suspected
        if (std::find(suspected_failed_.begin(), suspected_failed_.end(), node_id) != suspected_failed_.end())
        {
            suspected_failed_.erase(
                std::remove(suspected_failed_.begin(), suspected_failed_.end(), node_id),
                suspected_failed_.end());
            confirmed_failed_.push_back(node_id);
            node->is_alive = false;
        }
        else
        {
            // First time detecting failure
            suspected_failed_.push_back(node_id);
        }
    }
    else
    {
        // Node is healthy, remove from failure lists
        std::unique_lock lock(mutex_);
        suspected_failed_.erase(
            std::remove(suspected_failed_.begin(), suspected_failed_.end(), node_id),
            suspected_failed_.end());
        confirmed_failed_.erase(
            std::remove(confirmed_failed_.begin(), confirmed_failed_.end(), node_id),
            confirmed_failed_.end());
        node->is_alive = true;
    }
}

void FailureDetector::trigger_failover()
{
    std::shared_lock lock(mutex_);

    // Check if current leader is in failed list
    auto nodes = cluster_state_->get_all_nodes();
    for (const auto& node : nodes)
    {
        if (node->role == NodeRole::LEADER &&
            std::find(confirmed_failed_.begin(), confirmed_failed_.end(), node->id) != confirmed_failed_.end())
        {
            lock.unlock();

            // Leader is failed, this node should become leader
            auto self_node = cluster_state_->get_node(cluster_state_->get_self_id());
            if (self_node && self_node->id != node->id)
            {
                std::cout << "Leader " << node->id << " failed, promoting to leader" << std::endl;
                cluster_state_->become_leader();
            }
            return;
        }
    }
}
