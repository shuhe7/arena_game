# Authoritative Matchmaking Room Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Deliver a server-authoritative two-player matchmaking-to-room flow with deterministic automated tests and client-side matchmaking states.

**Architecture:** `GameServer`'s main `EventLoop` owns a `MatchmakingService`. The service owns the existing `MatchQueue`, uses `RoomService` to allocate rooms, and updates `SessionService` before returning match events. `GameServer` serializes those events into existing TCP frames; `LobbyScene` reacts only to server responses.

**Tech Stack:** C++17, CMake/CTest on CentOS 8, custom Reactor/epoll networking, Qt 6/qmake and QtTest on Windows.

**Spec:** `docs/superpowers/specs/2026-09-03-authoritative-matchmaking-room-design.md`

## Global Constraints

- Match state is mutated only on the `GameServer` main `EventLoop`; do not add threads or locks to matching state.
- Use existing length-prefixed framing and strict `GameMessages::encode` / `GameMessages::decode` calls.
- The matchmaking tick interval is exactly 100ms; room IDs are nonzero, monotonically increasing `uint64_t` values.
- Do not implement battle tick, movement, damage, rating settlement, cancellation, reconnect, database migration, or authentication changes.
- Commit source and build configuration only; do not commit generated binaries, build directories, moc files, or account data.

---

## File Structure

- `server/CMakeLists.txt`: C++17 game-core and game-server libraries plus CTest targets.
- `server/game/Room.{h,cc}`: room records and ID allocation only.
- `server/game/MatchmakingService.{h,cc}`: queue/session/room coordination without TCP.
- `server/game/GameServer.{h,cc}`: 100ms scheduling and protocol notification conversion.
- `server/tests/*.cc`: dependency-free assertion test executables.
- `client/ArenaClient/scenes/LobbyScene.{h,cc}`: visible lobby state transitions.
- `client/ArenaClient/mainwindow.cpp`: sends requests and registers response handlers.

### Task 1: M0 client baseline and disconnect cleanup

**Files:**
- Modify: `server/game/GameServer.cc:107-125`
- Verify and commit: `client/ArenaClient/mainwindow.cpp`, `client/ArenaClient/scenes/LobbyScene.{h,cc}`, `client/ArenaClient/scenes/LoginScene.cc`, `client/ArenaClient/tests/LobbySceneTest.{cc,pro}`

**Produces:** a committed five-case LobbyScene Qt baseline and a connection-close path that invokes `SessionService::remove` exactly once.

- [ ] **Step 1: Run the existing standalone Qt test**

Open `client/ArenaClient/tests/LobbySceneTest.pro` in Qt Creator with the application kit. Run **Clean**, **Run qmake**, then **Rebuild**. Expect five passing cases. Confirm `client/ArenaClient/ArenaClient.pro` does not include `tests/LobbySceneTest.cc`.

- [ ] **Step 2: Make the minimal disconnect correction**

In the disconnected branch of `GameServer::onConnection`, keep the queue cancellation in the `session != nullptr` block, remove its inner `sessionService_.remove(conn->id())`, and leave one unconditional `sessionService_.remove(conn->id())` before the `connections_.erase` lock block.

- [ ] **Step 3: Verify and commit two M0 units**

Run `git diff --check` and re-run the Qt project from Step 1. Stage and commit the client files with `完善大厅匹配请求状态`; then stage only `server/game/GameServer.cc` and commit it with `修复断线时的重复会话清理`.

### Task 2: Establish deterministic server tests

**Files:**
- Create: `server/CMakeLists.txt`, `server/tests/MatchQueueTest.cc`, `server/tests/MatchMessageTest.cc`
- Modify: `server/muduo/CMakeLists.txt`

**Consumes:** `MatchQueue::{join,cancel,tryMatch,size,contains}` and `GameMessages::{encode,decode}`.

**Produces:** CTest targets `match_queue_test` and `match_message_test` executable with `ctest --test-dir build/server --output-on-failure`.

- [ ] **Step 1: Write `MatchQueueTest.cc` first**

Use `assert` and a test-local `main`. Assert: a first join succeeds; duplicate user join fails; ELO 1000/1050 match at the same clock time; both matched users leave the queue; ELO 1000/1300 fails at join time then matches after ten seconds; `cancel` removes its user.

- [ ] **Step 2: Write `MatchMessageTest.cc` first**

Round-trip: accepted `MatchJoinResponse`; rejected `MatchJoinResponse` with `kInvalidState`; empty `MatchJoinRequest`; and `MatchFoundNotification{42, 7, "alice", 1280}`. Assert that a non-empty `MatchJoinRequest` payload fails strict decode.

- [ ] **Step 3: Add minimal CMake test wiring**

Create `server/CMakeLists.txt` with `CMAKE_CXX_STANDARD 17`, `enable_testing()`, an `arena_game_core` static library from `game/MatchQueue.cc` and `game/SessionService.cc`, two test executables, and two `add_test` entries. Replace the explicit `-std=c++11` flags in `server/muduo/CMakeLists.txt` with `CMAKE_CXX_STANDARD 17` and `CMAKE_CXX_STANDARD_REQUIRED ON`.

- [ ] **Step 4: Verify and commit**

On CentOS run `cmake -S server -B build/server`, `cmake --build build/server --parallel`, and `ctest --test-dir build/server --output-on-failure`. Expect both tests to pass. Commit the four files with `添加服务端匹配测试基础`.

### Task 3: Add the room domain model

**Files:**
- Create: `server/game/Room.h`, `server/game/Room.cc`, `server/tests/RoomServiceTest.cc`
- Modify: `server/CMakeLists.txt`

**Produces:**

```cpp
struct Room { uint64_t roomId_; uint32_t firstUserId_; uint32_t secondUserId_; };
class RoomService {
public:
    uint64_t create(uint32_t firstUserId, uint32_t secondUserId);
    const Room* find(uint64_t roomId) const;
private:
    uint64_t nextRoomId_ = 1;
    std::unordered_map<uint64_t, Room> rooms_;
};
```

- [ ] **Step 1: Write the failing `RoomServiceTest`**

Assert `create(7, 9)` returns 1, the next create returns 2, `find(1)` stores players 7 and 9, and `find(99)` is `nullptr`.

- [ ] **Step 2: Confirm the test fails, then implement only the stated interface**

Build with `cmake --build build/server --parallel`; expect `RoomService` missing. Add an unordered map-backed implementation with no player names, connections, locking, removal, battle state, or network code.

- [ ] **Step 3: Verify and commit**

Register `room_service_test`, reconfigure, build, and run `ctest --test-dir build/server -R room_service_test --output-on-failure`. Commit `Room.{h,cc}`, its test, and CMake with `添加匹配房间领域服务`.

### Task 4: Add a testable matchmaking coordinator

**Files:**
- Create: `server/game/MatchmakingService.h`, `server/game/MatchmakingService.cc`, `server/tests/MatchmakingServiceTest.cc`
- Modify: `server/CMakeLists.txt`

**Consumes:** `MatchQueue`, `RoomService`, mutable `SessionService`.

**Produces:**

```cpp
struct MatchFoundEvent { uint64_t roomId_; MatchTicket first_; MatchTicket second_; };
class MatchmakingService {
public:
    MatchmakingService(RoomService& rooms, SessionService& sessions);
    bool join(MatchTicket ticket);
    bool cancel(uint32_t userId);
    std::vector<MatchFoundEvent> tick(MatchTimePoint now);
private:
    MatchQueue queue_;
    RoomService& rooms_;
    SessionService& sessions_;
};
```

- [ ] **Step 1: Write the failing service tests**

Bind user 7 to connection 100 and user 9 to 200, queue `{7, 1000, now}` and `{9, 1050, now}`, call `tick(now)`, and assert one event, nonzero room ID, failed subsequent `cancel(7)`, and equal nonzero session room IDs. Bind only user 7, queue both tickets, and assert no event and no room.

- [ ] **Step 2: Confirm failure, then implement no-TCP coordination**

Build and expect the absent type error. In `tick`, repeat `queue_.tryMatch(now)` until empty. For each pair require both sessions and `roomId_ == 0`; otherwise emit no event. For valid pairs create a room, mark both sessions in it, and append the event.

- [ ] **Step 3: Verify and commit**

Reconfigure, build, and run the full CTest suite. Commit the two service files, test, and CMake with `添加权威匹配协调服务`.

### Task 5: Connect matching to GameServer and protocol

**Files:**
- Modify: `common/GameProtocol.h`, `server/game/GameServer.h`, `server/game/GameServer.cc`, `server/CMakeLists.txt`, `server/tests/MatchMessageTest.cc`

**Consumes:** `MatchmakingService::tick(MatchTimePoint)` and `MatchFoundEvent`.

**Produces:** `MSG_MATCH_FOUND_NTF = 0x0032` and `GameServer::processMatchmakingTick()`.

- [ ] **Step 1: Write the failing protocol assertion**

Add `assert(GameProtocol::MSG_MATCH_FOUND_NTF == 0x0032);` to `MatchMessageTest.cc`; build and expect compilation failure.

- [ ] **Step 2: Implement server integration**

Replace direct `MatchQueue matchQueue_` use with `RoomService roomService_` followed by `MatchmakingService matchmakingService_`. Replace `GameServer() = default` with a declared constructor, and define `GameServer::GameServer() : matchmakingService_(roomService_, sessionService_) {}` in `GameServer.cc`. In `init`, schedule `mainLoop_->runEvery(0.1, std::bind(&GameServer::processMatchmakingTick, this))`. The tick converts each event into two `MatchFoundNotification` values containing the same room ID and the other player's ID, name, and ELO; it sends each with `MSG_MATCH_FOUND_NTF`. `handleMatchJoin` and disconnect call `matchmakingService_.join` and `.cancel`.

Extend `server/CMakeLists.txt` with `add_subdirectory(muduo)` and an `arena_game_server` static library from `game/GameServer.cc`, `game/AccountRepository.cc`, and `db/ConfigMgr.cc`; link it with `arena_game_core` and `mymuduo`. This compile target verifies that the new GameServer member construction and timer integration are valid even before an application executable is introduced.

- [ ] **Step 3: Verify and commit**

Run the full CTest suite and `git diff --check`. Start the CentOS server and use two logged-in clients with ELO difference at most 100; both must receive one match notification within one tick after the second joins. Commit with `完成服务端匹配到房间通知`.

### Task 6: Make LobbyScene obey the server state

**Files:**
- Modify: `client/ArenaClient/scenes/LobbyScene.h`, `client/ArenaClient/scenes/LobbyScene.cc`, `client/ArenaClient/mainwindow.cpp`, `client/ArenaClient/tests/LobbySceneTest.cc`

**Produces:**

```cpp
void LobbyScene::showMatchmakingAccepted();
void LobbyScene::showMatchmakingRejected(const QString& message);
void LobbyScene::showMatchFound(uint64_t roomId, const QString& opponentName, uint32_t opponentElo);
```

- [ ] **Step 1: Write failing UI tests**

Assert `showMatchmakingAccepted()` displays `WAITING FOR OPPONENT...` with a disabled button; `showMatchmakingRejected("Already in matchmaking queue")` re-enables it and displays that exact text; `showMatchFound(42, "bob", 1260)` displays `MATCH FOUND // ROOM 42 // OPPONENT bob // ELO 1260` with a disabled button.

- [ ] **Step 2: Confirm failure, then implement state transitions**

Rebuild `LobbySceneTest.pro` and expect missing-method errors. Implement only its button enabled state and status string. In `MainWindow::initUi`, connect `matchRequested` to encoding `MatchJoinRequest{}` and `GameClient::sendMessage(MSG_MATCH_JOIN_REQ, payload)`. Register strict response handlers; malformed input displays `Malformed matchmaking response` and re-enables the button.

- [ ] **Step 3: Verify and commit**

Run the standalone Qt tests, then repeat the two-client smoke test. Confirm no client generates a room ID. Commit with `接入客户端匹配状态通知`.

### Task 7: M1 release verification

**Files:**
- Verify: all files from Tasks 1-6

- [ ] **Step 1: Run full automated checks**

On CentOS: run `cmake -S server -B build/server`, `cmake --build build/server --parallel`, and `ctest --test-dir build/server --output-on-failure`. In Qt Creator: Clean, Run qmake, Rebuild `LobbySceneTest.pro`.

- [ ] **Step 2: Check repository hygiene and record acceptance**

Run `git diff --check`, `git status --short`, and `git log --oneline -8`. Record the server and client commit hashes, two non-secret test account names, and the common room ID observed by both clients. Do not record passwords or `accounts.txt` contents.
