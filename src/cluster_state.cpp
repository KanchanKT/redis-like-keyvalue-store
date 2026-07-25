#include "cluster_state.h"

#include <mutex>

ClusterState::ClusterState(std::string self_id, int self_port)
    : self_id_(std::move(self_id))
    , self_port_(self_port)
    , current_term_(0)
    , leader_id_("")
{
    // Add self to the cluster
    add_node(self_id_, "localhost", self_port_);
}

void ClusterState::add_node(const std::string& node_id, const std::string& host, int port)
{
    std::unique_lock lock(mutex_);
    if (nodes_.find(node_id) == nodes_.end())
    {
        nodes_[node_id] = std::make_shared<ClusterNode>(node_id, host, port);
    }
}

void ClusterState::remove_node(const std::string& node_id)
{
    std::unique_lock lock(mutex_);
    nodes_.erase(node_id);
}

std::shared_ptr<ClusterNode> ClusterState::get_node(const std::string& node_id)
{
    std::shared_lock lock(mutex_);
    auto it = nodes_.find(node_id);
    if (it != nodes_.end())
    {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<ClusterNode>> ClusterState::get_all_nodes()
{
    std::shared_lock lock(mutex_);
    std::vector<std::shared_ptr<ClusterNode>> result;
    for (auto& [id, node] : nodes_)
    {
        result.push_back(node);
    }
    return result;
}

std::vector<std::shared_ptr<ClusterNode>> ClusterState::get_followers()
{
    std::shared_lock lock(mutex_);
    std::vector<std::shared_ptr<ClusterNode>> result;
    for (auto& [id, node] : nodes_)
    {
        if (id != self_id_)
        {
            result.push_back(node);
        }
    }
    return result;
}

uint64_t ClusterState::get_term() const
{
    std::shared_lock lock(mutex_);
    return current_term_;
}

void ClusterState::set_term(uint64_t term)
{
    std::unique_lock lock(mutex_);
    current_term_ = term;
}

bool ClusterState::is_leader() const
{
    std::shared_lock lock(mutex_);
    return leader_id_ == self_id_;
}

void ClusterState::become_leader()
{
    std::unique_lock lock(mutex_);
    leader_id_ = self_id_;
    for (auto& [id, node] : nodes_)
    {
        if (id != self_id_)
        {
            node->role = NodeRole::FOLLOWER;
        }
    }
    auto self = nodes_.find(self_id_);
    if (self != nodes_.end())
    {
        self->second->role = NodeRole::LEADER;
    }
}

void ClusterState::become_follower()
{
    std::unique_lock lock(mutex_);
    leader_id_ = "";
    auto self = nodes_.find(self_id_);
    if (self != nodes_.end())
    {
        self->second->role = NodeRole::FOLLOWER;
    }
}

std::string ClusterState::get_self_id() const
{
    std::shared_lock lock(mutex_);
    return self_id_;
}

void ClusterState::update_node_heartbeat(const std::string& node_id)
{
    std::unique_lock lock(mutex_);
    auto it = nodes_.find(node_id);
    if (it != nodes_.end())
    {
        it->second->update_heartbeat();
    }
}

std::vector<std::string> ClusterState::get_unhealthy_nodes(std::chrono::milliseconds heartbeat_timeout)
{
    std::shared_lock lock(mutex_);
    std::vector<std::string> unhealthy;
    for (const auto& [id, node] : nodes_)
    {
        if (!node->is_healthy(heartbeat_timeout))
        {
            unhealthy.push_back(id);
        }
    }
    return unhealthy;
}
