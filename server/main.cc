#include "game/GameServer.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    const std::string configPath = argc > 1 ? argv[1] : "server.conf";

    GameServer& server = GameServer::instance();
    if(!server.init(configPath))
    {
        std::cerr << "Failed to initialize server\n";
        return 1;
    }

    server.start();
    return 0;
}
