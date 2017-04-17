// Copyright 2015-2017 Directive Games Limited - All Rights Reserved

#include "DriftPrivatePCH.h"

#include "AndroidSecureStorage.h"


#if PLATFORM_ANDROID


AndroidSecureStorage::AndroidSecureStorage(const FString& productName, const FString& serviceName)
: productName_{ productName }
, serviceName_{ serviceName }
{
}


bool AndroidSecureStorage::SaveValue(const FString& key, const FString& value, bool overwrite)
{
    return false;
}


bool AndroidSecureStorage::GetValue(const FString& key, FString& value)
{
    return false;
}


#endif // PLATFORM_ANDROID
