// Copyright 2016-2017 Directive Games Limited - All Rights Reserved

#include "DriftUe4Plugin.h"
#include "ServerLoginGameSession.h"

#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"


void AServerLoginGameSession::RegisterServer()
{
    /**
     * The Engine will call this for servers during startup after
     * having successfully completed IOnlineIdentity::AutoLogin()
     * Fill out the various properties with the relevant data to
     * have this server session register with Drift.
     * Once registration is completed, the IOnlineSession::OnCreateSessionComplete
     * callback will be fired. The server should then prepare for accepting
     * clients, and finally call IOnlineSession::StartSession() to open the match
     * up to the match maker.
     */
    auto sessionInterface = Online::GetSessionInterface(GetWorld());
    if (sessionInterface.IsValid())
    {
        FOnlineSessionSettings Settings;
        Settings.NumPublicConnections = 2;
        Settings.bShouldAdvertise = true;
        Settings.bAllowJoinInProgress = false;
        Settings.bIsLANMatch = true;
        Settings.bUsesPresence = true;
        Settings.bAllowJoinViaPresence = true;

        sessionInterface->CreateSession(0, GameSessionName, Settings);
    }
}
