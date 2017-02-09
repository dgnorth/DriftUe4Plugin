// Copyright (c) 2017 Directive Games Limited - All Rights Reserved

#include "DriftUe4Plugin.h"
#include "MatchGameMode.h"

#include "OnlineSubsystemUtils.h"


void AMatchGameMode::BeginPlay()
{
    Super::BeginPlay();

    /**
     * Start the session and make it available to the match maker
     */
    if (IsRunningDedicatedServer())
    {
        auto sessionInterface = Online::GetSessionInterface(GetWorld());
        if (sessionInterface.IsValid())
        {
            sessionInterface->StartSession(GameSessionName);
        }
    }
}
