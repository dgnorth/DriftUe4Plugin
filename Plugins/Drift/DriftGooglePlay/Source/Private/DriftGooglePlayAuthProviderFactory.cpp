
#include "DriftGooglePlayPCH.h"

#include "DriftGooglePlayAuthProviderFactory.h"
#include "DriftGooglePlayAuthProvider.h"


FName FDriftGooglePlayAuthProviderFactory::GetAuthProviderName() const
{
	return FName(TEXT("GooglePlay"));
}


TUniquePtr<IDriftAuthProvider> FDriftGooglePlayAuthProviderFactory::GetAuthProvider()
{
	return MakeUnique<FDriftGooglePlayAuthProvider>();
}
