// Copyright (c) 2017 Directive Games Limited - All Rights Reserved

#pragma once

#include "Engine/GameInstance.h"

#include "DriftAPI.h"

#include "OnlineSessionInterface.h"

#include "DriftDemoGameInstance.generated.h"


class FOnlineSessionSearch;


UCLASS()
class UDriftDemoGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    // GameInstance overrides
    void Init() override;
    void Shutdown() override;

    // API
    UFUNCTION(BlueprintCallable, Category = "Drift|Matches")
    void StartMatchMaking();

    UFUNCTION(BlueprintCallable, Category = "Drift|Matches")
    void CancelMatchMaking();

    UFUNCTION(BlueprintCallable, Category = "Drift|Identitites")
    void LinkIdentity(const FString& credentialsType);

private:
    void HandleConnectionStateChanged(EDriftConnectionState state);
    void HandleLoginComplete(int32 LocalUserNum, bool success, const FUniqueNetId& UniqueId, const FString& Error);
    void HandleMatchMakingComplete(FName sessionName, bool success);
    void HandleCancelMatchMakingComplete(FName sessionName, bool success);
    void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type result);

private:
    FDelegateHandle matchMakingCompleteHandle;
    FDelegateHandle cancelMatchMakingCompleteHandle;
    FDelegateHandle joinSessionCompleteHandle;

    TSharedPtr<FOnlineSessionSearch> sessionSearch;
};
