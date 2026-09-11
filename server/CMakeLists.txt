cmake_minimum_required(VERSION 3.10)

project(arena_game_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(muduo)

find_package(Threads REQUIRED)

add_executable(arena_game_server
    main.cc
    db/ConfigMgr.cc
    game/AccountRepository.cc
    game/GameServer.cc
    game/MatchmakingService.cc
    game/MatchQueue.cc
    game/Room.cc
    game/SessionService.cc
    game/BattleService.cc
)

target_link_libraries(arena_game_server PRIVATE
    mymuduo
    Threads::Threads
    stdc++fs
)

set_target_properties(arena_game_server PROPERTIES
    BUILD_RPATH "${CMAKE_CURRENT_SOURCE_DIR}/muduo/lib"
)
