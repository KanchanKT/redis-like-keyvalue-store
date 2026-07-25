#include "cluster_node.h"

ClusterNode::ClusterNode(std::string node_id, std::string h, int p)
    : id(std::move(node_id))
    , host(std::move(h))
    , port(p)
    , role(NodeRole::FOLLOWER)
    , term(0)
    , is_alive(true)
    , last_heartbeat(std::chrono::steady_clock::now())
    , applied_index(0)
    , match_index(0)
    , next_index(0)
{
}

bool ClusterNode::is_healthy(std::chrono::milliseconds heartbeat_timeout) const
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_heartbeat);
    return elapsed < heartbeat_timeout;
}

void ClusterNode::update_heartbeat()
{
    last_heartbeat = std::chrono::steady_clock::now();
    is_alive = true;
}
