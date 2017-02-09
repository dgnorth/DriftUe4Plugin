// Copyright 2016-2017 Directive Games Limited - All Rights Reserved

#include "DriftUe4Plugin.h"
#include "ServerLoginGameMode.h"

#include "ServerLoginGameSession.h"

#include "OnlineSubsystemUtils.h"


TSubclassOf<AGameSession> AServerLoginGameMode::GetGameSessionClass() const
{
    /**
     * For automatic session registration, we must use a custom session class
     */
    return AServerLoginGameSession::StaticClass();
}


void AServerLoginGameMode::BeginPlay()
{
    Super::BeginPlay();

    /**
     * Register handling of completed session creation. Server backend login
     * and session registration is initiated automatically by the engine on
     * dedicated server startup.
     */
    auto sessionInterface = Online::GetSessionInterface(GetWorld());
    if (sessionInterface.IsValid())
    {
        createSessionCompleteHandle = sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &AServerLoginGameMode::HandleCreateSessionComplete);
    }
}


void AServerLoginGameMode::HandleCreateSessionComplete(FName SessionName, bool success)
{
    /**
    * Load the match map, or multiplayer lobby here, then call IOnlineSessionInterface::StartSession()
    * once ready to accept players.
    */
    UGameplayStatics::OpenLevel(this, TEXT("NativeMatch"));
}
