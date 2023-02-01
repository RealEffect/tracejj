#import "trace/system/Path.h"
#import <Foundation/Foundation.h>

fs::path GetExecutablePath()
{
    static fs::path pathExecutable;
    if (pathExecutable.empty())
    {
        NSString* path = [[NSBundle mainBundle] executablePath];
        const char* strPath= [path UTF8String];
        pathExecutable.assign(strPath);
    }
    return pathExecutable;
}

fs::path GetLogsPath()
{
    fs::path pathLog;
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
    NSString* libDirectory = [paths objectAtIndex:0];
    const char* strPath= [libDirectory UTF8String];
    pathLog.assign(strPath);
    pathLog /= "Logs";
#ifdef PRODUCT_PUBLISHER
    pathLog /= PRODUCT_PUBLISHER;
#endif
    pathLog /= GetExecutablePath().filename();
    return pathLog;
}
