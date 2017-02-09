// Copyright 2016-2017 Directive Games Limited - All Rights Reserved

#pragma once

#include "GameFramework/GameMode.h"
#include "ServerLoginGameMode.generated.h"


UCLASS()
class AServerLoginGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    // Actor overrides
    void BeginPlay() override;

    // GameModeBase overrides
    TSubclassOf<AGameSession> GetGameSessionClass() const override;

private:
    void HandleCreateSessionComplete(FName SessionName, bool success);

private:
    FDelegateHandle createSessionCompleteHandle;
};
