
#include "DriftSteamPCH.h"

#include "DriftSteamAuthProviderFactory.h"
#include "DriftSteamAuthProvider.h"


FName FDriftSteamAuthProviderFactory::GetAuthProviderName() const
{
	return FName(TEXT("Steam"));
}


TUniquePtr<IDriftAuthProvider> FDriftSteamAuthProviderFactory::GetAuthProvider()
{
	return MakeUnique<FDriftSteamAuthProvider>();
}
