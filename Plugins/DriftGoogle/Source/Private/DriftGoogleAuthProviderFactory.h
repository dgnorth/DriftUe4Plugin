
#pragma once


#include "IDriftAuthProviderFactory.h"


class FDriftGoogleAuthProviderFactory : public IDriftAuthProviderFactory
{
    FName GetAuthProviderName() const override;
    TUniquePtr<IDriftAuthProvider> GetAuthProvider() override;
};
