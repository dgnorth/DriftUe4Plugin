// Copyright 2016-2017 Directive Games Limited - All Rights Reserved

#pragma once

#include "DriftGoogleAuthProviderFactory.h"

#include "ModuleManager.h"


class FDriftGoogleModule : public IModuleInterface
{
public:
	FDriftGoogleModule();
    
    bool IsGameModule() const override
    {
        return true;
    }

    void StartupModule() override;
    void ShutdownModule() override;

private:
	FDriftGoogleAuthProviderFactory providerFactory;
};
