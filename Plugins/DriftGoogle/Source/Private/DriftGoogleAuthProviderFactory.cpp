
#include "DriftGooglePCH.h"

#include "DriftGoogleAuthProviderFactory.h"
#include "DriftGoogleAuthProvider.h"


FName FDriftGoogleAuthProviderFactory::GetAuthProviderName() const
{
	return FName(TEXT("Google"));
}


TUniquePtr<IDriftAuthProvider> FDriftGoogleAuthProviderFactory::GetAuthProvider()
{
	return MakeUnique<FDriftGoogleAuthProvider>();
}
