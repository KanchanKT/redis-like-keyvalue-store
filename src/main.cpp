#include "key_value_store.h"
#include "network_server.h"

int main()
{
    KeyValueStore store("wal.log");
    return run_tcp_server(store);
}
