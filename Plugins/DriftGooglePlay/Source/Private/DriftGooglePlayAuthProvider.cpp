
#include "DriftGooglePlayPCH.h"

#include "DriftGooglePlayAuthProvider.h"

#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemUtils.h"


DEFINE_LOG_CATEGORY_STATIC(LogDriftGooglePlay, Log, All);


FDriftGooglePlayAuthProvider::FDriftGooglePlayAuthProvider()
{
}


void FDriftGooglePlayAuthProvider::InitCredentials(InitCredentialsCallback callback)
{
    const auto localUserNum = 0;

    auto identityInterface = Online::GetIdentityInterface(nullptr, GOOGLEPLAY_SUBSYSTEM);
    if (identityInterface.IsValid())
    {
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
            loginCompleteDelegateHandle = identityInterface->AddOnLoginCompleteDelegate_Handle(localUserNum, FOnLoginCompleteDelegate::CreateSP(this, &FDriftGooglePlayAuthProvider::OnLoginComplete, callback));
            identityInterface->AutoLogin(localUserNum);
        }
    }
    else
    {
        UE_LOG(LogDriftGooglePlay, Error, TEXT("Failed to get online user identity interface"));
        callback(false);
    }
}


void FDriftGooglePlayAuthProvider::GetFriends(GetFriendsCallback callback)
{
    auto friendsInterface = Online::GetFriendsInterface(nullptr, GOOGLEPLAY_SUBSYSTEM);
    if (friendsInterface.IsValid())
    {
        friendsInterface->ReadFriendsList(0, TEXT("Default"), FOnReadFriendsListComplete::CreateLambda([this, callback](int32, bool success, const FString& listName, const FString& error)
        {
            if (success)
            {
                TArray<TSharedRef<FOnlineFriend>> friends;
                auto friendsInterface = Online::GetFriendsInterface(nullptr, GOOGLEPLAY_SUBSYSTEM);
                if (friendsInterface.IsValid())
                {
                    if (friendsInterface->GetFriendsList(0, TEXT("Default"), friends))
                    {
                        callback(true, friends);
                        return;
                    }
                    else
                    {
                        UE_LOG(LogDriftGooglePlay, Warning, TEXT("Failed to get friends list"));
                    }
                }
                else
                {
                    UE_LOG(LogDriftGooglePlay, Warning, TEXT("Failed to get online friends interface"));
                }
            }
            else
            {
                UE_LOG(LogDriftGooglePlay, Warning, TEXT("Failed to read friends list: \"%s\""), *error);
            }
            callback(false, TArray<TSharedRef<FOnlineFriend>>());
        }));
        return;
    }
    else
    {
        UE_LOG(LogDriftGooglePlay, Warning, TEXT("Failed to get online friends interface"));
    }
    callback(false, TArray<TSharedRef<FOnlineFriend>>());
}


void FDriftGooglePlayAuthProvider::FillProviderDetails(DetailsAppender appender) const
{/*
    appender(TEXT("steam_id"), steamID);
    appender(TEXT("ticket"), token);
    appender(TEXT("appid"), FString::Printf(TEXT("%d"), appID));*/
}


FString FDriftGooglePlayAuthProvider::GetNickname()
{
    auto identityInterface = Online::GetIdentityInterface(nullptr, GOOGLEPLAY_SUBSYSTEM);
    if (identityInterface.IsValid() && identityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn)
    {
        return identityInterface->GetPlayerNickname(0);
    }
    return TEXT("");
}


FString FDriftGooglePlayAuthProvider::ToString() const
{
    return L""; // FString::Printf(TEXT("Google ID: id=%s, appid=%d"), *steamID, appID);
}


void FDriftGooglePlayAuthProvider::GetAvatarUrl(GetAvatarUrlCallback callback)
{
    callback(L"");
}


void FDriftGooglePlayAuthProvider::OnLoginComplete(int32 localPlayerNum, bool success, const FUniqueNetId& userID, const FString& error, InitCredentialsCallback callback)
{
    if (success)
    {
        auto identityInterface = Online::GetIdentityInterface(nullptr, GOOGLEPLAY_SUBSYSTEM);
        if (identityInterface.IsValid())
        {
            identityInterface->ClearOnLoginCompleteDelegate_Handle(localPlayerNum, loginCompleteDelegateHandle);

            if (identityInterface->GetLoginStatus(localPlayerNum) == ELoginStatus::LoggedIn)
            {
                auto id = identityInterface->GetUniquePlayerId(localPlayerNum);
                if (id.IsValid())
                {
                    googleID = id->ToString();
                    callback(true);
                    return;
                }
                else
                {
                    UE_LOG(LogDriftGooglePlay, Error, TEXT("Failed to get unique player ID"));
                }
            }
            else
            {
                UE_LOG(LogDriftGooglePlay, Error, TEXT("Login status is not LoggedIn"));
            }
        }
        else
        {
            UE_LOG(LogDriftGooglePlay, Error, TEXT("Failed to get online user identity interface"));
        }
    }
    else
    {
        UE_LOG(LogDriftGooglePlay, Error, TEXT("Failed to login to Google Play: %s"), *error);
    }

    callback(false);
}
