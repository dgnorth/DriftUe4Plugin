
#pragma once


#include "IDriftAuthProviderFactory.h"


class FDriftGooglePlayAuthProviderFactory : public IDriftAuthProviderFactory
{
    FName GetAuthProviderName() const override;
    TUniquePtr<IDriftAuthProvider> GetAuthProvider() override;
};
