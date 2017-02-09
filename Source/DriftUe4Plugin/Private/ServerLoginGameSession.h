// Copyright 2016-2017 Directive Games Limited - All Rights Reserved

#pragma once

#include "GameFramework/GameSession.h"
#include "ServerLoginGameSession.generated.h"


UCLASS()
class AServerLoginGameSession : public AGameSession
{
    GENERATED_BODY()

public:
    // GameSession overrides
    void RegisterServer() override;
};
