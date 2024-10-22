#ifdef __linux__
#include "Path.h"
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <memory>

fs::path GetExecutablePath()
{
    std::vector<char> buffer(FILENAME_MAX);
    int len = readlink("/proc/self/exe", buffer.data(), buffer.size());
    if (len >= 0)
    {
        buffer[len] = '\0';
        return fs::path(buffer.data());
    }
    return fs::path();
}

fs::path GetDefaultLogPath()
{
    fs::path pathLog;
    passwd pwd{};
    passwd* result = nullptr;
    long lBuffer = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (lBuffer <= 0)
    {
        lBuffer = 16 * 1024;
    }
    std::unique_ptr<char[]> pBuffer(new char[lBuffer]);
    getpwuid_r(getuid(), &pwd, pBuffer.get(), lBuffer, &result);
    if (result == nullptr || pwd.pw_dir == nullptr)
    {
        pathLog.assign("/var/log/");
    }
    else
    {
        pathLog.assign(pwd.pw_dir);
    }
#ifdef PRODUCT_PUBLISHER
    pathLog /= "." PRODUCT_PUBLISHER;
#endif
    pathLog /= GetExecutablePath().filename();
    return pathLog;
}

#endif
