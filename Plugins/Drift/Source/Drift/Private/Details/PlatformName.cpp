
#include "DriftPrivatePCH.h"

#include "PlatformName.h"


namespace details
{

const TCHAR* GetPlatformName()
{
    static const TCHAR platform_name[] =
#if PLATFORM_WINDOWS
    L"Windows";
#elif PLATFORM_PS4
    L"PS4";
#elif PLATFORM_XBOXONE
    L"XBOX1";
#elif PLATFORM_MAC
    L"MAC";
#elif PLATFORM_IOS
    return FIOSPlatformMisc::GetDefaultDeviceProfileName();
#elif PLATFORM_ANDROID
    L"Android";
#elif PLATFORM_WINRT_ARM
    L"WinRT";
#elif PLATFORM_WINRT
    L"WinRT";
#elif PLATFORM_HTML5
    L"HTML5";
#elif PLATFORM_LINUX
    L"Linux";
#else
#error Unknown Platform
#endif
    return platform_name;
}

} // namespace details
