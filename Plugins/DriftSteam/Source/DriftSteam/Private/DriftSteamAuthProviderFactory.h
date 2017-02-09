
#pragma once


#include "IDriftAuthProviderFactory.h"


class FDriftSteamAuthProviderFactory : public IDriftAuthProviderFactory
{
    FName GetAuthProviderName() const override;
    TUniquePtr<IDriftAuthProvider> GetAuthProvider() override;
};
