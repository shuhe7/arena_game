#include "ConfigMgr.h"
#include "../muduo/Logger.h"

#include <fstream>

ConfigMgr& ConfigMgr::instance()
{
    static ConfigMgr mgr;
    return mgr;
}

bool ConfigMgr::load(const std::string& path)
{
    std::fstream file(path);
    if(!file.is_open())
    {
        LOG_FATAL("[ConfigMgr] Cannot open config file: %s\n", path);
    }

    std::string line;
    while(std::getline(file, line))
    {
        line.erase(line.find_last_not_of(" \r\t") + 1);
        if(line.empty()) { continue; }

        size_t eq = line.find("=");
        if(eq == std::string::npos) { continue; }
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        data_[key] = val;
    }
    return true;
}

bool ConfigMgr::getInt(const std::string& key, int* val) const
{
    auto it = data_.find(key);
    if(it == data_.end())
    {
        LOG_FATAL("[ConfigMgr] Cannot find key: %s\n", key);
    }
    *val = std::stoi(it->second);
    return true;
}

std::string ConfigMgr::get(const std::string& key, const std::string& val) const 
{
    auto it = data_.find(key);
    return it != data_.end() ? it->second : val;
}