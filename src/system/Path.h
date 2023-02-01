#pragma once

#ifdef USE_GHC_FILESYSTEM
#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

fs::path GetExecutablePath();
fs::path GetLogsPath();
