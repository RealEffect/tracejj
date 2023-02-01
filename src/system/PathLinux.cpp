#ifdef __linux__
#include <unistd.h>
#include "system/Path.h"

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

fs::path GetLogsPath()
{
    fs::path pathLog;
    pathLog.assign("/var/log/");
#ifdef PRODUCT_PUBLISHER
    pathLog /= PRODUCT_PUBLISHER;
#endif
    pathLog /= GetExecutablePath().filename();
    return pathLog;
}

#endif
