
#include "DriftOculusPCH.h"

#include "DriftOculusModule.h"

#include "IPluginManager.h"
#include "Features/IModularFeatures.h"


IMPLEMENT_MODULE(FDriftOculusModule, DriftOculus)


FDriftOculusModule::FDriftOculusModule()
{
}


void FDriftOculusModule::StartupModule()
{
    FModuleManager::Get().LoadModuleChecked("Drift");
    IModularFeatures::Get().RegisterModularFeature(TEXT("DriftAuthProviderFactory"), &providerFactory);
}


void FDriftOculusModule::ShutdownModule()
{
    IModularFeatures::Get().UnregisterModularFeature(TEXT("DriftAuthProviderFactory"), &providerFactory);
}
