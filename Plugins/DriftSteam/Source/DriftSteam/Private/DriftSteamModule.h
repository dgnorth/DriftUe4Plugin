// Copyright 2016-2017 Directive Games Limited - All Rights Reserved

#pragma once

#include "DriftSteamAuthProviderFactory.h"

#include "ModuleManager.h"


class FDriftSteamModule : public IModuleInterface
{
public:
	FDriftSteamModule();
    
    bool IsGameModule() const override
    {
        return true;
    }

    void StartupModule() override;
    void ShutdownModule() override;

private:
	FDriftSteamAuthProviderFactory providerFactory;
};
