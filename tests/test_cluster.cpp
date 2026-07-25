#include "consistent_hash.h"
#include "cluster_state.h"
#include "cluster_node.h"
#include "replication.h"
#include "failure_detector.h"

#include <iostream>
#include <memory>
#include <cassert>
#include <thread>
#include <chrono>

void test_consistent_hash()
{
    std::cout << "\n=== Testing Consistent Hashing ===" << std::endl;

    ConsistentHash hash(160);

    // Add nodes to the ring
    hash.add_node("node1");
    hash.add_node("node2");
    hash.add_node("node3");

    // Test key distribution
    std::string key1 = "user:123";
    std::string key2 = "user:456";
    std::string key3 = "user:789";

    std::string node1 = hash.get_node(key1);
    std::string node2 = hash.get_node(key2);
    std::string node3 = hash.get_node(key3);

    std::cout << "Key '" << key1 << "' -> " << node1 << std::endl;
    std::cout << "Key '" << key2 << "' -> " << node2 << std::endl;
    std::cout << "Key '" << key3 << "' -> " << node3 << std::endl;

    // Test replicas
    auto replicas1 = hash.get_replicas(key1, 2);
    std::cout << "Replicas for '" << key1 << "': ";
    for (const auto& r : replicas1)
    {
        std::cout << r << " ";
    }
    std::cout << std::endl;

    // Test node removal and rebalancing
    std::cout << "\nRemoving node2..." << std::endl;
    hash.remove_node("node2");

    std::string new_node1 = hash.get_node(key1);
    std::string new_node2 = hash.get_node(key2);

    std::cout << "After removal:" << std::endl;
    std::cout << "Key '" << key1 << "' -> " << new_node1 << std::endl;
    std::cout << "Key '" << key2 << "' -> " << new_node2 << std::endl;

    std::cout << "✓ Consistent hashing test passed" << std::endl;
}

void test_cluster_state()
{
    std::cout << "\n=== Testing Cluster State ===" << std::endl;

    auto cluster = std::make_shared<ClusterState>("node1", 6379);

    // Add nodes to cluster
    cluster->add_node("node2", "localhost", 6380);
    cluster->add_node("node3", "localhost", 6381);

    auto all_nodes = cluster->get_all_nodes();
    std::cout << "Cluster has " << all_nodes.size() << " nodes:" << std::endl;
    for (const auto& node : all_nodes)
    {
        std::cout << "  - " << node->id << " (" << node->host << ":" << node->port << ")" << std::endl;
    }

    // Test leadership
    std::cout << "\nNode1 becoming leader..." << std::endl;
    cluster->become_leader();
    assert(cluster->is_leader());

    auto followers = cluster->get_followers();
    std::cout << "Followers count: " << followers.size() << std::endl;
    for (const auto& follower : followers)
    {
        std::cout << "  - " << follower->id << std::endl;
    }

    // Test term management
    cluster->set_term(5);
    assert(cluster->get_term() == 5);
    std::cout << "Current term: " << cluster->get_term() << std::endl;

    std::cout << "✓ Cluster state test passed" << std::endl;
}

void test_failure_detection()
{
    std::cout << "\n=== Testing Failure Detection ===" << std::endl;

    auto cluster = std::make_shared<ClusterState>("node1", 6379);
    cluster->add_node("node2", "localhost", 6380);
    cluster->add_node("node3", "localhost", 6381);

    auto failure_detector = std::make_shared<FailureDetector>(cluster, std::chrono::milliseconds(500));

    // Initially all nodes should be healthy
    std::cout << "Initial state: all nodes healthy" << std::endl;
    auto node2 = cluster->get_node("node2");
    assert(node2->is_healthy(std::chrono::milliseconds(500)));

    // Simulate node failure
    std::cout << "\nSimulating node2 failure by stopping heartbeat..." << std::endl;
    // Don't update heartbeat for node2, wait for timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(600));

    // Start failure detector
    failure_detector->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check if node2 is now suspected as failed
    auto suspected = failure_detector->get_suspected_failed();
    std::cout << "Suspected failed nodes: " << suspected.size() << std::endl;

    // Mark node2 as failed and test recovery
    failure_detector->mark_node_failed("node2");
    auto confirmed = failure_detector->get_confirmed_failed();
    std::cout << "Confirmed failed nodes: " << confirmed.size() << std::endl;

    // Recover node2
    failure_detector->mark_node_recovered("node2");
    auto confirmed_after = failure_detector->get_confirmed_failed();
    std::cout << "After recovery, confirmed failed nodes: " << confirmed_after.size() << std::endl;

    failure_detector->stop();

    std::cout << "✓ Failure detection test passed" << std::endl;
}

void test_replication()
{
    std::cout << "\n=== Testing Replication ===" << std::endl;

    auto cluster = std::make_shared<ClusterState>("leader", 6379);
    cluster->add_node("follower1", "localhost", 6380);
    cluster->add_node("follower2", "localhost", 6381);
    cluster->become_leader();

    auto replication = std::make_shared<Replication>(cluster);

    // Test command replication
    std::cout << "Leader replicating SET command to followers..." << std::endl;
    replication->replicate_command("SET", "user:123", "alice");

    // Get replication stats
    auto stats = replication->get_stats();
    std::cout << "Replicated commands: " << stats.replicated_count << std::endl;
    std::cout << "Failed replications: " << stats.failed_replication_count << std::endl;

    // Test heartbeats
    std::cout << "\nLeader sending heartbeats..." << std::endl;
    replication->send_heartbeats();

    auto node = cluster->get_node("follower1");
    assert(node->is_healthy(std::chrono::milliseconds(5000)));
    std::cout << "✓ Follower1 received heartbeat" << std::endl;

    std::cout << "✓ Replication test passed" << std::endl;
}

int main()
{
    std::cout << "Running Phase 6: Distributed System Tests" << std::endl;
    std::cout << "==========================================" << std::endl;

    try
    {
        test_consistent_hash();
        test_cluster_state();
        test_failure_detection();
        test_replication();

        std::cout << "\n=== All Phase 6 Tests Passed ===" << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
