// Copyright (c) 2017 Directive Games Limited - All Rights Reserved

#pragma once

#include "DriftUe4PluginGameModeBase.h"
#include "NativeLoginGameMode.generated.h"


struct FPlayerAuthenticatedInfo;


UCLASS()
class ANativeLoginGameMode : public ADriftUe4PluginGameModeBase
{
    GENERATED_BODY()

public:
    // Actor overrides
    void BeginPlay() override;

private:
    void HandlePlayerAuthenticated(bool success, const FPlayerAuthenticatedInfo& info);
};
