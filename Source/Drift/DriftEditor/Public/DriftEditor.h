// Copyright 2015-2017 Directive Games Limited - All Rights Reserved

#pragma once

#include "ModuleManager.h"


class FDriftEditor : public IModuleInterface
{

public:
    void StartupModule() override;
    void ShutdownModule() override;
};
