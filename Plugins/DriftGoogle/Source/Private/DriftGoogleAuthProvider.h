#pragma once

#include "IDriftAuthProvider.h"


class FDriftGoogleAuthProvider : public IDriftAuthProvider
{
public:
    FDriftGoogleAuthProvider();

    FString GetProviderName() const override { return TEXT("google"); }
    void InitCredentials(InitCredentialsCallback callback) override;
    void GetFriends(GetFriendsCallback callback) override;
    void GetAvatarUrl(GetAvatarUrlCallback callback) override;
    void FillProviderDetails(DetailsAppender appender) const override;
    FString GetNickname() override;

    FString ToString() const override;

private:
};
