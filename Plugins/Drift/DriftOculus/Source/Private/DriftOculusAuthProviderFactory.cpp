
#include "DriftOculusPCH.h"

#include "DriftOculusAuthProviderFactory.h"
#include "DriftOculusAuthProvider.h"


FName FDriftOculusAuthProviderFactory::GetAuthProviderName() const
{
	return FName(TEXT("oculus"));
}


TUniquePtr<IDriftAuthProvider> FDriftOculusAuthProviderFactory::GetAuthProvider()
{
	return MakeUnique<FDriftOculusAuthProvider>();
}
