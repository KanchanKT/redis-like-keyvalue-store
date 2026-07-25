#pragma once

#include <cstdint>
#include <functional>
#include <string>

class WriteAheadLog
{
public:
    explicit WriteAheadLog(std::string file_path);
    ~WriteAheadLog();

    bool append_set(const std::string& key, const std::string& value);
    bool append_delete(const std::string& key);
    bool append_clear();

    bool replay(const std::function<void(const std::string&, const std::string&)>& on_set,
                const std::function<void(const std::string&)>& on_delete,
                const std::function<void()>& on_clear);

private:
    enum class RecordType : std::uint8_t
    {
        Set = 1,
        Delete = 2,
        Clear = 3,
    };

    bool open_file();
    bool append_record(RecordType type,
                       const std::string& key,
                       const std::string& value);
    bool flush_and_sync();
    static bool write_exact(int fd, const char* data, std::size_t size);
    static bool read_exact(int fd, char* data, std::size_t size);
    static bool write_u32(int fd, std::uint32_t value);
    static bool read_u32(int fd, std::uint32_t& value);

    int fd_;
    std::string file_path_;
};
