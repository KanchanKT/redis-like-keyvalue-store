#pragma once

#include <string>
#include <unordered_map>

class Snapshot
{
public:
    explicit Snapshot(std::string file_path);

    bool save(const std::unordered_map<std::string, std::string>& data);

    bool load(std::unordered_map<std::string, std::string>& data);

    bool exists() const;

private:
    std::string file_path_;
};
