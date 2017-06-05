
#include "DriftGooglePCH.h"

#include "DriftGoogleModule.h"

#include "IPluginManager.h"
#include "Features/IModularFeatures.h"


IMPLEMENT_MODULE(FDriftGoogleModule, DriftGoogle)


FDriftGoogleModule::FDriftGoogleModule()
{
}


void FDriftGoogleModule::StartupModule()
{
    FModuleManager::Get().LoadModuleChecked("Drift");
    IModularFeatures::Get().RegisterModularFeature(TEXT("DriftAuthProviderFactory"), &providerFactory);
}


void FDriftGoogleModule::ShutdownModule()
{
    IModularFeatures::Get().UnregisterModularFeature(TEXT("DriftAuthProviderFactory"), &providerFactory);
}
