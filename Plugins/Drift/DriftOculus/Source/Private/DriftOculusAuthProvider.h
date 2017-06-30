
#pragma once

#include "IDriftAuthProvider.h"

#include "OnlineIdentityInterface.h"
#include "OVR_Message.h"


class FDriftOculusAuthProvider : public IDriftAuthProvider
{
public:
    FDriftOculusAuthProvider();

    // IDriftAuthProvider
    FString GetProviderName() const override { return TEXT("oculus"); }
    void InitCredentials(InitCredentialsCallback callback) override;
    void GetFriends(GetFriendsCallback callback) override;
	void GetAvatarUrl(GetAvatarUrlCallback callback) override;
    void FillProviderDetails(DetailsAppender appender) const override;
    FString GetNickname() override;

    FString ToString() const override;

private:
    void ValidateOwnership(InitCredentialsCallback callback);
    void GetNonceForIdentityCheck(InitCredentialsCallback callback);

    void OnReadFriendsListComplete(int32, bool success, const FString& listName, const FString& error, GetFriendsCallback callback);
    void OnLoginComplete(int32 localPlayerNum, bool success, const FUniqueNetId& userID, const FString& error, InitCredentialsCallback callback);
    void OnGetNonceComplete(ovrMessageHandle handle, bool isError, InitCredentialsCallback callback);
    void OnValidationComplete(const FUniqueNetId& uniqueID, EUserPrivileges::Type privilege, uint32 result, InitCredentialsCallback callback);
    void OnGetAvatarUrlComplete(ovrMessageHandle handle, bool isError, GetAvatarUrlCallback callback);

    void TriggerCallbackIfLogonComplete(InitCredentialsCallback callback);

private:
    FDelegateHandle loginCompleteDelegateHandle;

    FString oculusID;
    FString nonce;

    bool validated{ false };
};
