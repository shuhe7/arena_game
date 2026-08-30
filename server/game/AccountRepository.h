#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <cstddef>

struct Account
{
    uint32_t userId_ = 0;
    std::string userName_;
    uint32_t elo_ = 1000;
};

enum class AccountResult : uint8_t
{
    kOk = 0,
    kDuplicateUser,
    kInvalidCredentials,
    kInvalidInput,
    kStorageError,
};

class AccountRepository
{
public:
    explicit AccountRepository(std::string filePath);

    bool load();

    AccountResult Register(const std::string& userName, const std::string& password, Account& outAccount);
    AccountResult Verify(const std::string& userName, const std::string& password, Account& outAccount) const;

    std::size_t size() const;
private:
    struct AccountRecord
    {
        Account account_;
        std::string password_;
    };

    bool save() const;
    bool isValidInput(const std::string& userName, const std::string& password) const;

    std::string filePath_;
    uint32_t nextUserId_ = 1;
    std::unordered_map<std::string, AccountRecord> accounts_;
};