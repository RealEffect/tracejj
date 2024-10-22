#ifdef _WIN32

#include "Path.h"
#include <array>
#include <Windows.h>
#include <UserEnv.h>
#include <Shlobj.h>

fs::path GetExecutablePath()
{
    std::array<wchar_t, MAX_PATH> strExeFileName{0};
    ::GetModuleFileNameW(NULL, strExeFileName.data(), static_cast<DWORD>(strExeFileName.size()));
    return fs::path(strExeFileName.data());
}

fs::path GetDefaultLogPath()
{
    fs::path pathLog;
    if (pathLog.empty())
    {
        LPWSTR strPath = nullptr;
        const auto result = ::SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &strPath);
        const auto pathExec = GetExecutablePath();
        if (result == S_OK)
        {
            pathLog.assign(strPath);
            ::CoTaskMemFree(strPath);
        }
        else
        {
            // see https://devblogs.microsoft.com/oldnewthing/20160601-00/?p=93555
            // see https://learn.microsoft.com/en-us/windows/win32/api/userenv/nf-userenv-loaduserprofilew
            pathLog = pathExec;
            pathLog.remove_filename();
            pathLog /= L"Logs";
        }
#ifdef PRODUCT_PUBLISHER
        pathLog /= PRODUCT_PUBLISHER;
#endif
        pathLog /= pathExec.filename().replace_extension(L"");
    }
    return pathLog;
}

#endif
