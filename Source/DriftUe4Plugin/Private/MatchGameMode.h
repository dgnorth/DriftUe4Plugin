// Copyright (c) 2017 Directive Games Limited - All Rights Reserved

#pragma once

#include "DriftUe4PluginGameModeBase.h"
#include "MatchGameMode.generated.h"


UCLASS()
class AMatchGameMode : public ADriftUe4PluginGameModeBase
{
    GENERATED_BODY()
public:
    // Actor overrides
    void BeginPlay() override;
};
