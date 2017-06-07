// Copyright 2016-2017 Directive Games Limited - All Rights Reserved

#pragma once

#include "DriftGooglePlayAuthProviderFactory.h"

#include "ModuleManager.h"


class FDriftGooglePlayModule : public IModuleInterface
{
public:
	FDriftGooglePlayModule();
    
    bool IsGameModule() const override
    {
        return true;
    }

    void StartupModule() override;
    void ShutdownModule() override;

private:
	FDriftGooglePlayAuthProviderFactory providerFactory;
};
