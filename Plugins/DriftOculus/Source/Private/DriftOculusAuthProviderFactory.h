
#pragma once


#include "IDriftAuthProviderFactory.h"


class FDriftOculusAuthProviderFactory : public IDriftAuthProviderFactory
{
    FName GetAuthProviderName() const override;
    TUniquePtr<IDriftAuthProvider> GetAuthProvider() override;
};
