
#include "DriftSteamPCH.h"

#include "DriftSteamAuthProvider.h"

#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemUtils.h"
#include "Http.h"
#include "JsonUtilities.h"


DEFINE_LOG_CATEGORY_STATIC(LogDriftSteam, Log, All);


static const FString steamApiKey(TEXT("ENTER-YOUR-STEAM-API-KEY-HERE"));


FDriftSteamAuthProvider::FDriftSteamAuthProvider()
{
}


void FDriftSteamAuthProvider::InitCredentials(InitCredentialsCallback callback)
{
    const auto localUserNum = 0;

    /**
     * Steam is already logged in through the Steam client when the game starts, and the login
     * code never initiates a new login, it only checks the current status. Thus there's no point
     * in trying to recover if the player is not logged in at this point.
     */
    auto identityInterface = Online::GetIdentityInterface(nullptr, STEAM_SUBSYSTEM);
    if (identityInterface.IsValid() && identityInterface->GetLoginStatus(localUserNum) == ELoginStatus::LoggedIn)
    {
        auto id = identityInterface->GetUniquePlayerId(localUserNum);
        if (id.IsValid())
        {
            token = identityInterface->GetAuthToken(localUserNum);
            auto subsystem = Online::GetSubsystem(nullptr, STEAM_SUBSYSTEM);
            if (subsystem != nullptr)
            {
                appID = FCString::Atoi(*subsystem->GetAppId());
                steamID = id->ToString();
                callback(true);
                return;
            }
        }
    }
    else
    {
        UE_LOG(LogDriftSteam, Error, TEXT("Failed to login to Steam"));
    }

    callback(false);
}


void FDriftSteamAuthProvider::GetFriends(GetFriendsCallback callback)
{
    auto friendsInterface = Online::GetFriendsInterface(nullptr, STEAM_SUBSYSTEM);
    if (friendsInterface.IsValid())
    {
        friendsInterface->ReadFriendsList(0, TEXT("Default"), FOnReadFriendsListComplete::CreateLambda([this, callback](int32, bool success, const FString& listName, const FString& error)
        {
            if (success)
            {
                TArray<TSharedRef<FOnlineFriend>> friends;
                auto friendsInterface = Online::GetFriendsInterface(nullptr, STEAM_SUBSYSTEM);
                if (friendsInterface.IsValid())
                {
                    if (friendsInterface->GetFriendsList(0, TEXT("Default"), friends))
                    {
                        callback(true, friends);
                        return;
                    }
                    else
                    {
                        UE_LOG(LogDriftSteam, Warning, TEXT("Failed to get friends list"));
                    }
                }
                else
                {
                    UE_LOG(LogDriftSteam, Warning, TEXT("Failed to get online friends interface"));
                }
            }
            else
            {
                UE_LOG(LogDriftSteam, Warning, TEXT("Failed to read friends list: \"%s\""), *error);
            }
            callback(false, TArray<TSharedRef<FOnlineFriend>>());
        }));
        return;
    }
    else
    {
        UE_LOG(LogDriftSteam, Warning, TEXT("Failed to get online friends interface"));
    }
    callback(false, TArray<TSharedRef<FOnlineFriend>>());
}


void FDriftSteamAuthProvider::FillProviderDetails(DetailsAppender appender) const
{
    appender(TEXT("steam_id"), steamID);
    appender(TEXT("ticket"), token);
    appender(TEXT("appid"), FString::Printf(TEXT("%d"), appID));
}


FString FDriftSteamAuthProvider::GetNickname()
{
    auto identityInterface = Online::GetIdentityInterface(nullptr, STEAM_SUBSYSTEM);
    if (identityInterface.IsValid() && identityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn)
    {
        return identityInterface->GetPlayerNickname(0);
    }
    return TEXT("");
}


FString FDriftSteamAuthProvider::ToString() const
{
    return FString::Printf(TEXT("Steam ID: id=%s, appid=%d"), *steamID, appID);
}


void FDriftSteamAuthProvider::GetAvatarUrl(GetAvatarUrlCallback callback)
{
	auto url = FString::Printf(TEXT("http://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key=%s&steamids=%s"), *steamApiKey, *steamID);
	
	auto request = FHttpModule::Get().CreateRequest();
	request->SetURL(url);
	request->SetVerb(TEXT("GET"));
	request->OnProcessRequestComplete().BindLambda([this, callback](FHttpRequestPtr request, FHttpResponsePtr response, bool succeeded)
	{
		FString imageUrl;

		if (succeeded)
		{			
			auto contentString = response->GetContentAsString();
			FSteamPlayersSummaryResponse parsedResponse;

			if (FJsonObjectConverter::JsonObjectStringToUStruct(contentString, &parsedResponse, 0, 0) && 
				parsedResponse.response.players.Num() == 1)
			{
				imageUrl = parsedResponse.response.players[0].avatarfull;
				UE_LOG(LogDriftSteam, Log, TEXT("Avatar url for the current steam user: %s"), *imageUrl);
			}
			else
			{
				UE_LOG(LogDriftSteam, Error, TEXT("Failed to parse response string '%s'"), *contentString);
			}
		}
		else
		{
			UE_LOG(LogDriftSteam, Error, TEXT("Failed to query player summary"));
		}

		callback(imageUrl);
	});
	request->ProcessRequest();
}
