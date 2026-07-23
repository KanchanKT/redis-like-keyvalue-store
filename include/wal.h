#pragma once

#include <functional>
#include <string>

class WriteAheadLog
{
public:
    explicit WriteAheadLog(std::string file_path);

    bool append_set(const std::string& key, const std::string& value);
    bool append_delete(const std::string& key);
    bool append_clear();

    bool replay(const std::function<void(const std::string&, const std::string&)>& on_set,
                const std::function<void(const std::string&)>& on_delete,
                const std::function<void()>& on_clear);

private:
    std::string file_path_;
};
