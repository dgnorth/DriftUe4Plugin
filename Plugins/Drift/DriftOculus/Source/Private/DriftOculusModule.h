// Copyright 2016-2017 Directive Games Limited - All Rights Reserved

#pragma once

#include "DriftOculusAuthProviderFactory.h"

#include "ModuleManager.h"


class FDriftOculusModule : public IModuleInterface
{
public:
    FDriftOculusModule();
    
    bool IsGameModule() const override
    {
        return true;
    }

    void StartupModule() override;
    void ShutdownModule() override;

private:
	FDriftOculusAuthProviderFactory providerFactory;
};
