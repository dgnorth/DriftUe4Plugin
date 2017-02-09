// Copyright (c) 2017 Directive Games Limited - All Rights Reserved

#include "DriftUe4Plugin.h"
#include "DriftDemoGameInstance.h"

#include "DriftAPI.h"
#include "DriftUtils.h"

#include "OnlineSubsystemUtils.h"


void UDriftDemoGameInstance::Init()
{
    Super::Init();

    if (auto drift = FDriftWorldHelper(GetWorld()).GetInstance())
    {
        drift->OnConnectionStateChanged().AddUObject(this, &UDriftDemoGameInstance::HandleConnectionStateChanged);
    }

    if (IsRunningDedicatedServer())
    {
        return;
    }

    auto identityInterface = Online::GetIdentityInterface(GetWorld());
    if (identityInterface.IsValid())
    {
        identityInterface->OnLoginCompleteDelegates[0].AddUObject(this, &UDriftDemoGameInstance::HandleLoginComplete);
    }

    auto sessionInterface = Online::GetSessionInterface(GetWorld());
    if (sessionInterface.IsValid())
    {
        matchMakingCompleteHandle = sessionInterface->OnMatchmakingCompleteDelegates.AddUObject(this, &UDriftDemoGameInstance::HandleMatchMakingComplete);
        cancelMatchMakingCompleteHandle = sessionInterface->OnCancelMatchmakingCompleteDelegates.AddUObject(this, &UDriftDemoGameInstance::HandleCancelMatchMakingComplete);
        joinSessionCompleteHandle = sessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UDriftDemoGameInstance::HandleJoinSessionComplete);
    }
}


void UDriftDemoGameInstance::Shutdown()
{
    /**
     * Unreal unfortunately offers no hook that we can use to let the plugin handle this automatically
     */
    if (auto drift = FDriftWorldHelper(GetWorld()).GetInstance())
    {
        drift->Shutdown();
    }

    Super::Shutdown();
}


void UDriftDemoGameInstance::StartMatchMaking()
{
    if (sessionSearch.IsValid())
    {
        UE_LOG(LogDriftDemo, Warning, TEXT("Cannot start another match search while another is in progress"));

        return;
    }

    auto sessionInterface = Online::GetSessionInterface(GetWorld());
    if (sessionInterface.IsValid())
    {
        sessionSearch = MakeShareable(new FOnlineSessionSearch{});
        TArray<TSharedRef<const FUniqueNetId>> localPlayers;
        localPlayers.Add(GetLocalPlayerByIndex(0)->GetCachedUniqueNetId().ToSharedRef());
        auto temp = sessionSearch.ToSharedRef();
        if (sessionInterface->StartMatchmaking(localPlayers, GameSessionName, FOnlineSessionSettings{}, temp))
        {
        }
        else
        {
        }
    }
}


void UDriftDemoGameInstance::CancelMatchMaking()
{
    if (!sessionSearch.IsValid())
    {
        UE_LOG(LogDriftDemo, Warning, TEXT("There is no match search to cancel"));

        return;
    }

    auto sessionInterface = Online::GetSessionInterface(GetWorld());
    if (sessionInterface.IsValid())
    {
        if (!sessionInterface->CancelMatchmaking(0, GameSessionName))
        {
            HandleCancelMatchMakingComplete(GameSessionName, false);
        }
    }
}


void UDriftDemoGameInstance::HandleConnectionStateChanged(EDriftConnectionState state)
{
    UE_LOG(LogDriftDemo, Log, TEXT("Connection State: %d"), (uint8)state);
}


void UDriftDemoGameInstance::HandleLoginComplete(int32 LocalUserNum, bool success, const FUniqueNetId& UniqueId, const FString& Error)
{
    if (IsRunningDedicatedServer())
    {
        return;
    }

    if (success)
    {
        /**
        * The UniqueNetId must be cached on the local player and player state
        * for sessions to work. It's not clear why this is not done automatically
        * by the engine.
        */
        if (auto localPlayer = GetLocalPlayerByIndex(LocalUserNum))
        {
            auto uniqueId = localPlayer->GetUniqueNetIdFromCachedControllerId();
            if (auto playerController = localPlayer->GetPlayerController(GetWorld()))
            {
                if (auto playerState = playerController->PlayerState)
                {
                    localPlayer->SetCachedUniqueNetId(uniqueId);
                    playerState->SetUniqueId(uniqueId);
                }
            }
        }
    }
}


void UDriftDemoGameInstance::HandleMatchMakingComplete(FName sessionName, bool success)
{
    if (success && sessionSearch.IsValid() && sessionSearch->SearchResults.Num() > 0)
    {
        if (sessionSearch->SearchState == EOnlineAsyncTaskState::Done)
        {
            UE_LOG(LogDriftDemo, Log, TEXT("Found a match"));

            /**
             * Normally you'd want to keep the found match somewhere, while letting the player
             * know it has been matched, fade out the UI, etc, and only then join the session.
             */
            auto match = sessionSearch->SearchResults[0];

            if (match.Session.SessionInfo.IsValid())
            {
                auto sessionInterface = Online::GetSessionInterface(GetWorld());
                if (sessionInterface.IsValid())
                {
                    sessionInterface->JoinSession(0, GameSessionName, match);
                }
                else
                {
                    UE_LOG(LogDriftDemo, Error, TEXT("Failed to get SessionInterface"));
                }
            }
            else
            {
                UE_LOG(LogDriftDemo, Error, TEXT("Found match session is invalid"));
            }
        }
    }
    else
    {
        sessionSearch.Reset();
    }
}


void UDriftDemoGameInstance::HandleCancelMatchMakingComplete(FName sessionName, bool success)
{
    if (success)
    {
        sessionSearch.Reset();
    }
}


void UDriftDemoGameInstance::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type result)
{
    /**
     * Once the session has been joined, find the server URL and travel to it.
     */
    if (result == EOnJoinSessionCompleteResult::Success)
    {
        auto sessionInterface = Online::GetSessionInterface(GetWorld());
        if (sessionInterface.IsValid())
        {
            sessionSearch.Reset();

            FString serverUrl;
            if (sessionInterface->GetResolvedConnectString(GameSessionName, serverUrl))
            {
                APlayerController* controller = UGameplayStatics::GetPlayerController(this, 0);
                if (controller)
                {
                    controller->ClientTravel(serverUrl, TRAVEL_Absolute);
                }
                else
                {
                    UE_LOG(LogDriftDemo, Error, TEXT("Found no controller for travelling to match"));
                }
            }
            else
            {
                UE_LOG(LogDriftDemo, Error, TEXT("Failed to get the connection url from the match"));
            }
        }
        else
        {
            UE_LOG(LogDriftDemo, Error, TEXT("Failed to get SessionInterface"));
        }
    }
    else
    {
        UE_LOG(LogDriftDemo, Error, TEXT("JoinSession failed"));
    }
}
