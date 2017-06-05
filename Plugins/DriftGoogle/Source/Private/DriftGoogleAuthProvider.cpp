
#include "DriftGooglePCH.h"

#include "DriftGoogleAuthProvider.h"

#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemUtils.h"
#include "Http.h"
#include "JsonUtilities.h"


DEFINE_LOG_CATEGORY_STATIC(LogDriftGoogle, Log, All);


FDriftGoogleAuthProvider::FDriftGoogleAuthProvider()
{
}


void FDriftGoogleAuthProvider::InitCredentials(InitCredentialsCallback callback)
{
    const auto localUserNum = 0;

    /**
     * Google is already logged in through the Google client when the game starts, and the login
     * code never initiates a new login, it only checks the current status. Thus there's no point
     * in trying to recover if the player is not logged in at this point.
     */
    auto identityInterface = Online::GetIdentityInterface(nullptr, GOOGLE_SUBSYSTEM);
    if (identityInterface.IsValid() && identityInterface->GetLoginStatus(localUserNum) == ELoginStatus::LoggedIn)
    {
        auto id = identityInterface->GetUniquePlayerId(localUserNum);
        if (id.IsValid())
        {
            //token = identityInterface->GetAuthToken(localUserNum);
            auto subsystem = Online::GetSubsystem(nullptr, GOOGLE_SUBSYSTEM);
            if (subsystem != nullptr)
            {
                //appID = FCString::Atoi(*subsystem->GetAppId());
                //steamID = id->ToString();
                callback(true);
                return;
            }
        }
    }
    else
    {
        UE_LOG(LogDriftGoogle, Error, TEXT("Failed to login to Google"));
    }

    callback(false);
}


void FDriftGoogleAuthProvider::GetFriends(GetFriendsCallback callback)
{
    auto friendsInterface = Online::GetFriendsInterface(nullptr, GOOGLE_SUBSYSTEM);
    if (friendsInterface.IsValid())
    {
        friendsInterface->ReadFriendsList(0, TEXT("Default"), FOnReadFriendsListComplete::CreateLambda([this, callback](int32, bool success, const FString& listName, const FString& error)
        {
            if (success)
            {
                TArray<TSharedRef<FOnlineFriend>> friends;
                auto friendsInterface = Online::GetFriendsInterface(nullptr, GOOGLE_SUBSYSTEM);
                if (friendsInterface.IsValid())
                {
                    if (friendsInterface->GetFriendsList(0, TEXT("Default"), friends))
                    {
                        callback(true, friends);
                        return;
                    }
                    else
                    {
                        UE_LOG(LogDriftGoogle, Warning, TEXT("Failed to get friends list"));
                    }
                }
                else
                {
                    UE_LOG(LogDriftGoogle, Warning, TEXT("Failed to get online friends interface"));
                }
            }
            else
            {
                UE_LOG(LogDriftGoogle, Warning, TEXT("Failed to read friends list: \"%s\""), *error);
            }
            callback(false, TArray<TSharedRef<FOnlineFriend>>());
        }));
        return;
    }
    else
    {
        UE_LOG(LogDriftGoogle, Warning, TEXT("Failed to get online friends interface"));
    }
    callback(false, TArray<TSharedRef<FOnlineFriend>>());
}


void FDriftGoogleAuthProvider::FillProviderDetails(DetailsAppender appender) const
{/*
    appender(TEXT("steam_id"), steamID);
    appender(TEXT("ticket"), token);
    appender(TEXT("appid"), FString::Printf(TEXT("%d"), appID));*/
}


FString FDriftGoogleAuthProvider::GetNickname()
{
    auto identityInterface = Online::GetIdentityInterface(nullptr, GOOGLE_SUBSYSTEM);
    if (identityInterface.IsValid() && identityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn)
    {
        return identityInterface->GetPlayerNickname(0);
    }
    return TEXT("");
}


FString FDriftGoogleAuthProvider::ToString() const
{
    return L""; // FString::Printf(TEXT("Google ID: id=%s, appid=%d"), *steamID, appID);
}


void FDriftGoogleAuthProvider::GetAvatarUrl(GetAvatarUrlCallback callback)
{
    callback(L"");
}
