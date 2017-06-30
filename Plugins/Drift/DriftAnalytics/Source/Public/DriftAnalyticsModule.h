// Copyright 2015-2017 Directive Games Limited - All Rights Reserved

#pragma once

#include "Analytics.h"
#include "IAnalyticsProviderModule.h"


class FDriftAnalyticsModule : public IAnalyticsProviderModule
{
public:
	TSharedPtr<IAnalyticsProvider> CreateAnalyticsProvider(const FAnalyticsProviderConfigurationDelegate& GetConfigValue) const override;

private:
	void StartupModule() override;
	void ShutdownModule() override;
};
