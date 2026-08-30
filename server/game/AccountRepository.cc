#include "AccountRepository.h"

#include <fstream>
#include <experimental/filesystem>
#include <system_error>
#include <cctype>
#include <cstdio>
#include <limits>
#include <utility>
#include <iostream>

AccountRepository::AccountRepository(std::string filePath)
    : filePath_(std::move(filePath))
{}

bool AccountRepository::load()
{
    std::ifstream input(filePath_);
    if (!input.is_open())
    {
        std::error_code error;
        const bool exists = std::experimental::filesystem::exists(filePath_, error);

        if (!exists && !error)
        {
            return true;
        }

        return false;
    }


    std::unordered_map<std::string, AccountRecord> loadedAccounts;
    uint32_t nextUserId = 1;

    std::string userName;
    std::string password;
    uint32_t userId = 0;
    uint32_t elo = 0;

    while(input >> userName >> password >> userId >> elo)
    {
        if(!isValidInput(userName, password) || userId == 0)
        {
            return false;
        }

        AccountRecord record;
        record.account_.userId_ = userId;
        record.account_.userName_ = userName;
        record.account_.elo_ = elo;
        record.password_ = password;

        const auto result = loadedAccounts.emplace(userName, std::move(record));
        if(!result.second)
        {
            return false;
        }

        if(userId == std::numeric_limits<uint32_t>::max())
        {
            nextUserId = 0;
        }
        else if(nextUserId != 0 && userId >= nextUserId)
        {
            nextUserId = userId + 1;
        }
    }

    if(!input.eof())
    {
        return false;
    }

    accounts_ = std::move(loadedAccounts);
    nextUserId_ = nextUserId;
    return true;
}

AccountResult AccountRepository::Register(const std::string &userName, const std::string &password, Account &outAccount)
{
    if(!isValidInput(userName, password))
    {
        return AccountResult::kInvalidInput;
    }

    if(accounts_.find(userName) != accounts_.end())
    {
        return AccountResult::kDuplicateUser;
    }

    if(nextUserId_ == 0)
    {
        return AccountResult::kStorageError;
    }

    AccountRecord record;
    record.account_.userId_ = nextUserId_;
    record.account_.userName_ = userName;
    record.account_.elo_ = 1000;
    record.password_ = password;

    const auto inserted = accounts_.emplace(userName, std::move(record));
    if(!inserted.second)
    {
        return AccountResult::kDuplicateUser;
    }

    if(!save())
    {
        accounts_.erase(userName);
        return AccountResult::kStorageError;
    }

    outAccount = inserted.first->second.account_;

    if(nextUserId_ == std::numeric_limits<uint32_t>::max())
    {
        nextUserId_ = 0;
    }
    else
    {
        ++nextUserId_;
    }

    return AccountResult::kOk;
}
AccountResult AccountRepository::Verify(const std::string &userName, const std::string &password, Account &outAccount) const
{
    if(!isValidInput(userName, password))
    {
        return AccountResult::kInvalidInput;
    }

    const auto it = accounts_.find(userName);
    if(it == accounts_.end() || it->second.password_ != password)
    {
        return AccountResult::kInvalidCredentials;
    }

    outAccount = it->second.account_;
    return AccountResult::kOk;
}

std::size_t AccountRepository::size() const
{
    return accounts_.size();
}

bool AccountRepository::save() const
{
    const std::string tempPath = filePath_ + ".tmp";

    std::ofstream output(tempPath, std::ios::trunc);
    if(!output.is_open())
    {
        return false;
    }

    for(const auto& pair : accounts_)
    {
        const AccountRecord& record = pair.second;
        output << record.account_.userName_ << ' ' << record.password_ << ' ' << record.account_.userId_ << ' ' << record.account_.elo_ << '\n';        
    }

    output.flush();
    if(!output.good())
    {
        output.close();
        std::remove(tempPath.c_str());
        return false;
    }

    output.close();

    if(std::rename(tempPath.c_str(), filePath_.c_str()) != 0)
    {
        std::remove(tempPath.c_str());
        return false;
    }

    return true;
}
bool AccountRepository::isValidInput(const std::string &userName, const std::string &password) const
{
    if(userName.empty() || userName.size() > 32 || password.empty() || password.size() > 128)
    {
        return false;
    }

    for (unsigned char ch : userName)
    {
        if (std::isspace(ch))
        {
            return false;
        }
    }

    for (unsigned char ch : password)
    {
        if (std::isspace(ch))
        {
            return false;
        }
    }

    return true;    
}