
#include "DriftOculusPCH.h"

#include "DriftOculusAuthProvider.h"

#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemOculus.h"

#include "OVR_Platform.h"
#include "Misc/SecureHash.h"


#ifndef OCULUS_SUBSYSTEM
#define OCULUS_SUBSYSTEM FName(TEXT("Oculus"))
#endif


DEFINE_LOG_CATEGORY_STATIC(LogDriftOculus, Log, All);


FDriftOculusAuthProvider::FDriftOculusAuthProvider()
{
    
}


void FDriftOculusAuthProvider::InitCredentials(InitCredentialsCallback callback)
{
    /**
     * Oculus will try to auto login at game start, but if the login fails, we don't know why
     * at this point, so we connect our callback and try to login again.
     */
    auto identityInterface = Online::GetIdentityInterface(nullptr, OCULUS_SUBSYSTEM);
    if (identityInterface.IsValid())
    {
        const auto localUserNum = 0;

        if (identityInterface->GetLoginStatus(localUserNum) == ELoginStatus::LoggedIn)
        {
            auto id = identityInterface->GetUniquePlayerId(localUserNum);
            if (id.IsValid())
            {
                OnLoginComplete(localUserNum, true, *id.Get(), TEXT(""), callback);
            }
            else
            {
                callback(false);
            }
        }
        else
        {
            loginCompleteDelegateHandle = identityInterface->AddOnLoginCompleteDelegate_Handle(localUserNum, FOnLoginCompleteDelegate::CreateSP(this, &FDriftOculusAuthProvider::OnLoginComplete, callback));
            identityInterface->AutoLogin(localUserNum);
        }
    }
    else
    {
        callback(false);
    }
}


void FDriftOculusAuthProvider::GetFriends(GetFriendsCallback callback)
{
    auto friendsInterface = Online::GetFriendsInterface(nullptr, OCULUS_SUBSYSTEM);
    if (friendsInterface.IsValid())
    {
        friendsInterface->ReadFriendsList(0, TEXT("Default"), FOnReadFriendsListComplete::CreateLambda([this, callback](int32, bool success, const FString& listName, const FString& error)
        {
            if (success)
            {
                TArray<TSharedRef<FOnlineFriend>> friends;
                auto friendsInterface = Online::GetFriendsInterface(nullptr, OCULUS_SUBSYSTEM);
                if (friendsInterface.IsValid())
                {
                    if (friendsInterface->GetFriendsList(0, TEXT("Default"), friends))
                    {
                        callback(true, friends);
                        return;
                    }
                    else
                    {
                        UE_LOG(LogDriftOculus, Warning, TEXT("Failed to get friends list"));
                    }
                }
                else
                {
                    UE_LOG(LogDriftOculus, Warning, TEXT("Failed to get online friends interface"));
                }
            }
            else
            {
                UE_LOG(LogDriftOculus, Warning, TEXT("Failed to read friends list: \"%s\""), *error);
            }
            callback(false, TArray<TSharedRef<FOnlineFriend>>());
        }));
        return;
    }
    else
    {
        UE_LOG(LogDriftOculus, Warning, TEXT("Failed to get online friends interface"));
    }
    callback(false, TArray<TSharedRef<FOnlineFriend>>());
}


void FDriftOculusAuthProvider::FillProviderDetails(DetailsAppender appender) const
{
    static const FString salt = TEXT("429160df-c623-4580-be3c-98e748d148d4");

    appender(TEXT("provisional"), TEXT("true"));
    appender(TEXT("username"), *oculusID);
    // This is temporary until we can get the nonce from the API
    appender(TEXT("password"), *FMD5::HashAnsiString(*(oculusID + salt)));
    appender(TEXT("user_id"), *oculusID);
    appender(TEXT("nonce"), *nonce);
}


FString FDriftOculusAuthProvider::GetNickname()
{
    auto identityInterface = Online::GetIdentityInterface(nullptr, OCULUS_SUBSYSTEM);
    if (identityInterface.IsValid() && identityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn)
    {
        return identityInterface->GetPlayerNickname(0);
    }
    else
    {
        UE_LOG(LogDriftOculus, Warning, TEXT("Failed to get online nickname"));
    }

    return TEXT("");
}


FString FDriftOculusAuthProvider::ToString() const
{
    return FString::Printf(TEXT("Oculus ID: id=%s"), *oculusID);
}


void FDriftOculusAuthProvider::OnLoginComplete(int32 localPlayerNum, bool success, const FUniqueNetId& userID, const FString& error, InitCredentialsCallback callback)
{
    if (success)
    {
        auto identityInterface = Online::GetIdentityInterface(nullptr, OCULUS_SUBSYSTEM);
        if (identityInterface.IsValid())
        {
            identityInterface->ClearOnLoginCompleteDelegate_Handle(localPlayerNum, loginCompleteDelegateHandle);

            if (identityInterface->GetLoginStatus(localPlayerNum) == ELoginStatus::LoggedIn)
            {
                auto id = identityInterface->GetUniquePlayerId(localPlayerNum);
                if (id.IsValid())
                {
                    oculusID = id->ToString();
                    GetNonceForIdentityCheck(callback);
                    ValidateOwnership(callback);
                    return;
                }
                else
                {
                    UE_LOG(LogDriftOculus, Error, TEXT("Failed to get unique player ID"));
                }
            }
            else
            {
                UE_LOG(LogDriftOculus, Error, TEXT("Login status is not LoggedIn"));
            }
        }
        else
        {
            UE_LOG(LogDriftOculus, Error, TEXT("Failed to get online user identity interface"));
        }
    }
    else
    {
        UE_LOG(LogDriftOculus, Error, TEXT("Failed to login to Oculus: %s"), *error);
    }

    callback(false);
}


void FDriftOculusAuthProvider::ValidateOwnership(InitCredentialsCallback callback)
{
    auto identityInterface = Online::GetIdentityInterface(nullptr, OCULUS_SUBSYSTEM);
    if (identityInterface.IsValid())
    {
        auto Unused = new FUniqueNetIdString("UNUSED");
        identityInterface->GetUserPrivilege(
            *Unused,
            EUserPrivileges::CanPlay,
            IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateSP(this, &FDriftOculusAuthProvider::OnValidationComplete, callback)
        );
    }
    else
    {
        UE_LOG(LogDriftOculus, Error, TEXT("Failed to get online user identity interface"));

        callback(false);
    }
}


void FDriftOculusAuthProvider::OnValidationComplete(const FUniqueNetId& uniqueID, EUserPrivileges::Type privilege, uint32 result, InitCredentialsCallback callback)
{
    if (result == 0)
    {
        validated = true;
        TriggerCallbackIfLogonComplete(callback);
    }
    else
    {
        UE_LOG(LogDriftOculus, Error, TEXT("Failed to validate app ownership through Oculus store"));

        callback(false);
    }
}


void FDriftOculusAuthProvider::GetNonceForIdentityCheck(InitCredentialsCallback callback)
{
    auto subsystem = static_cast<FOnlineSubsystemOculus*>(Online::GetSubsystem(nullptr, OCULUS_SUBSYSTEM));
    if (subsystem != nullptr)
    {
        subsystem->AddRequestDelegate(::ovr_User_GetUserProof(), FOculusMessageOnCompleteDelegate::CreateSP(this, &FDriftOculusAuthProvider::OnGetNonceComplete, callback));
    }
    else
    {
        UE_LOG(LogDriftOculus, Error, TEXT("Failed to get Oculus subsystem instance for identity validation"));

        callback(false);
    }
}


void FDriftOculusAuthProvider::OnGetNonceComplete(ovrMessageHandle handle, bool isError, InitCredentialsCallback callback)
{
    if (isError)
    {
        UE_LOG(LogDriftOculus, Error, TEXT("Failed to get nonce for backend validation"));

        callback(false);
    }
    else
    {
        auto message = ::ovr_Message_GetUserProof(handle);
        if (message)
        {
            nonce = ::ovr_UserProof_GetNonce(message);

            TriggerCallbackIfLogonComplete(callback);
        }
        else
        {
            UE_LOG(LogDriftOculus, Error, TEXT("Failed to get decode nonce for backend validation"));

            callback(false);
        }
    }
}


void FDriftOculusAuthProvider::TriggerCallbackIfLogonComplete(InitCredentialsCallback callback)
{
    if (validated && !nonce.IsEmpty())
    {
        callback(true);
    }
}


void FDriftOculusAuthProvider::GetAvatarUrl(GetAvatarUrlCallback callback)
{
    if (auto oss = Online::GetSubsystem(nullptr, OCULUS_SUBSYSTEM))
    {
        auto ossOculus = static_cast<FOnlineSubsystemOculus*>(oss);
        ossOculus->AddRequestDelegate(::ovr_User_GetLoggedInUser(), FOculusMessageOnCompleteDelegate::CreateSP(this, &FDriftOculusAuthProvider::OnGetAvatarUrlComplete, callback));
    }
    else
    {
        UE_LOG(LogDriftOculus, Error, TEXT("Failed to get Oculus subsystem for avatar retrieval"));

        callback(TEXT(""));
    }    
}


void FDriftOculusAuthProvider::OnGetAvatarUrlComplete(ovrMessageHandle handle, bool isError, GetAvatarUrlCallback callback)
{
    if (isError)
    {
        UE_LOG(LogDriftOculus, Error, TEXT("Request for `ovr_User_GetLoggedInUser` failed"));

        callback(TEXT(""));
    }
    else
    {
        auto user = ::ovr_Message_GetUser(handle);
        FString imageUrl = ::ovr_User_GetImageUrl(user);
        UE_LOG(LogDriftOculus, Log, TEXT("Avatar url for the current oculus user: %s"), *imageUrl);
        callback(imageUrl);
    }
}
