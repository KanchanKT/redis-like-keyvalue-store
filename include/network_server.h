#pragma once

#include <string>

class KeyValueStore;

int run_tcp_server(KeyValueStore& store, int port = 6379);
