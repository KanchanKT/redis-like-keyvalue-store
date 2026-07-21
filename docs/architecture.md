Section 1

# Redis-Like Key-Value Store

## Overview

This project aims to build a Redis-inspired distributed key-value store from scratch in modern C++.

The goal is not to clone Redis feature-for-feature, but to understand and implement the core building blocks of a scalable distributed storage system.

The project will evolve incrementally:
- Version 1: Single-threaded in-memory key-value store
- Version 2: TCP server
- Version 3: Multi-threaded request processing
- Version 4: Write-Ahead Logging (WAL)
- Version 5: Persistent storage
- Version 6: Distributed replication

Section 2 — Functional Requirements

## Functional Requirements

The database should support:

- SET key value
- GET key
- DELETE key
- EXISTS key
- SIZE
- CLEAR

Section 3 — Non-Functional Requirements

## Non Functional Requirements

- Low latency
- High throughput
- Thread safety
- Extensible architecture
- Modular design

Section 4 — Version 1 Scope

## Version 1

In-memory storage

No networking

No persistence

Single threaded

Section 5 — High-Level Architecture

+------------+
| CLI Client |
+------------+
       |
       v
+----------------+
| Command Parser |
+----------------+
       |
       v
+----------------+
| KeyValueStore  |
+----------------+
       |
       v
unordered_map



