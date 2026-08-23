#pragma once

#include <string>
#include <cstdint>
#include <cstring>

struct PlayerConnection 
{
    uint64_t connId_ = 0;
    uint32_t userId_ = 0;
    std::string userName_;
    bool isOnline_ = false;
    uint32_t elo_ = 1000;
};
