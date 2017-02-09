
#include "DriftSteamPCH.h"

#include "DriftSteamModule.h"

#include "IPluginManager.h"
#include "Features/IModularFeatures.h"


IMPLEMENT_MODULE(FDriftSteamModule, DriftSteam)


FDriftSteamModule::FDriftSteamModule()
{
}


void FDriftSteamModule::StartupModule()
{
    FModuleManager::Get().LoadModuleChecked("Drift");
    IModularFeatures::Get().RegisterModularFeature(TEXT("DriftAuthProviderFactory"), &providerFactory);
}


void FDriftSteamModule::ShutdownModule()
{
    IModularFeatures::Get().UnregisterModularFeature(TEXT("DriftAuthProviderFactory"), &providerFactory);
}
