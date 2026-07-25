#include "key_value_store.h"
#include "network_server.h"
#include "cluster_state.h"
#include "consistent_hash.h"
#include "replication.h"
#include "failure_detector.h"

#include <iostream>
#include <memory>

int main(int argc, char* argv[])
{
    // Parse command line arguments
    int port = 6379;
    std::string node_id = "node1";

    if (argc >= 2)
    {
        port = std::stoi(argv[1]);
    }

    if (argc >= 3)
    {
        node_id = argv[2];
    }

    std::cout << "Starting Redis-like server" << std::endl;
    std::cout << "  Node ID: " << node_id << std::endl;
    std::cout << "  Port: " << port << std::endl;

    // Initialize distributed components
    auto cluster_state = std::make_shared<ClusterState>(node_id, port);
    auto consistent_hash = std::make_shared<ConsistentHash>(160);
    auto replication = std::make_shared<Replication>(cluster_state);
    auto failure_detector = std::make_shared<FailureDetector>(cluster_state);

    // Add this node to the cluster
    cluster_state->add_node(node_id, "localhost", port);
    consistent_hash->add_node(node_id);

    // Start background services
    replication->start();
    failure_detector->start();

    // Initialize the key-value store with persistence
    KeyValueStore store("wal.log", "snapshot.bin");

    // Start the server (this is a blocking call)
    int result = run_tcp_server(store, port);

    // Cleanup
    replication->stop();
    failure_detector->stop();

    return result;
}
