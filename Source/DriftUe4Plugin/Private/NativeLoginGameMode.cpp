// Copyright (c) 2017 Directive Games Limited - All Rights Reserved

#include "DriftUe4Plugin.h"
#include "NativeLoginGameMode.h"

#include "DriftAPI.h"
#include "DriftUtils.h"


void ANativeLoginGameMode::BeginPlay()
{
    /**
     * Begin player authentication as soon as possible
     */
    if (auto drift = FDriftWorldHelper(GetWorld()).GetInstance())
    {
        drift->OnPlayerAuthenticated().AddUObject(this, &ANativeLoginGameMode::HandlePlayerAuthenticated);
        drift->AuthenticatePlayer();
    }
}


void ANativeLoginGameMode::HandlePlayerAuthenticated(bool success, const FPlayerAuthenticatedInfo& info)
{
    /**
     * When the player has finished logging in, travel to the main menu level.
     * It's not strictly necessary to have the login and main map separated, but
     * it reduces the need for the main map to have to deal with both states.
     */
    if (success)
    {
        UGameplayStatics::OpenLevel(this, TEXT("NativeMenu"));
    }
    else
    {
        UE_LOG(LogDriftDemo, Error, TEXT("Login failed: %s"), *info.error);
    }
}
