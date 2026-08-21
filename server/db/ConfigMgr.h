#pragma once

#include <unordered_map>
#include <string>

class ConfigMgr
{
public:
    static ConfigMgr& instance();

    bool load(const std::string& path);

    bool getInt(const std::string& key, int* val) const;

private:
    ConfigMgr() = default;

    std::unordered_map<std::string, std::string> data_;
};