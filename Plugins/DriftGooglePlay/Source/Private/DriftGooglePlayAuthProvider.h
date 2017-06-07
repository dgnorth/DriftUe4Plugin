#pragma once

#include "IDriftAuthProvider.h"

#include "OnlineIdentityInterface.h"


class FDriftGooglePlayAuthProvider : public IDriftAuthProvider
{
public:
    FDriftGooglePlayAuthProvider();

    FString GetProviderName() const override { return TEXT("googleplay"); }
    void InitCredentials(InitCredentialsCallback callback) override;
    void GetFriends(GetFriendsCallback callback) override;
    void GetAvatarUrl(GetAvatarUrlCallback callback) override;
    void FillProviderDetails(DetailsAppender appender) const override;
    FString GetNickname() override;

    FString ToString() const override;

private:
    void OnLoginComplete(int32 localPlayerNum, bool success, const FUniqueNetId& userID, const FString& error, InitCredentialsCallback callback);

private:
    FDelegateHandle loginCompleteDelegateHandle;

    FString googleID;
};
