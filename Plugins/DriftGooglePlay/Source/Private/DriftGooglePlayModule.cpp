
#include "DriftGooglePlayPCH.h"

#include "DriftGooglePlayModule.h"

#include "IPluginManager.h"
#include "Features/IModularFeatures.h"


IMPLEMENT_MODULE(FDriftGooglePlayModule, DriftGooglePlay)


FDriftGooglePlayModule::FDriftGooglePlayModule()
{
}


void FDriftGooglePlayModule::StartupModule()
{
    FModuleManager::Get().LoadModuleChecked("Drift");
    IModularFeatures::Get().RegisterModularFeature(TEXT("DriftAuthProviderFactory"), &providerFactory);
}


void FDriftGooglePlayModule::ShutdownModule()
{
    IModularFeatures::Get().UnregisterModularFeature(TEXT("DriftAuthProviderFactory"), &providerFactory);
}
