// Copyright 2015-2017 Directive Games Limited - All Rights Reserved

#include "DriftAnalyticsPrivatePCH.h"

#include "DriftAnalyticsModule.h"
#include "DriftAnalyticsProvider.h"


IMPLEMENT_MODULE(FDriftAnalyticsModule, DriftAnalytics)


TSharedPtr<IAnalyticsProvider> FDriftAnalyticsModule::CreateAnalyticsProvider(const FAnalyticsProviderConfigurationDelegate& GetConfigValue) const
{
	return MakeShareable(new FDriftAnalyticsProvider());
}


void FDriftAnalyticsModule::StartupModule()
{
    FModuleManager::Get().LoadModuleChecked("Drift");
}


void FDriftAnalyticsModule::ShutdownModule()
{

}
