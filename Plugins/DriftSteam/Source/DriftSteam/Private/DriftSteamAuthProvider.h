#pragma once

#include "IDriftAuthProvider.h"
#include "DriftSteamAuthProvider.generated.h"

USTRUCT()
struct FSteamPlayerSummary
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FString avatar;

	UPROPERTY()
	FString avatarfull;

	UPROPERTY()
	FString avatarmedium;

	UPROPERTY()
	FString personaname;

	UPROPERTY()
	FString profileurl;

	UPROPERTY()
	FString realname;
};


USTRUCT()
struct FSteamPlayersSummary
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FSteamPlayerSummary> players;
};


USTRUCT()
struct FSteamPlayersSummaryResponse
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FSteamPlayersSummary response;
};


class FDriftSteamAuthProvider : public IDriftAuthProvider
{
public:
    FDriftSteamAuthProvider();

    FString GetProviderName() const override { return TEXT("steam"); }
    void InitCredentials(InitCredentialsCallback callback) override;
    void GetFriends(GetFriendsCallback callback) override;
    void GetAvatarUrl(GetAvatarUrlCallback callback) override;
    void FillProviderDetails(DetailsAppender appender) const override;
    FString GetNickname() override;

    FString ToString() const override;
    
private:
    FString steamID;
    FString token;
    int32 appID = -1;
};
