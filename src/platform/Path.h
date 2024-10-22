#pragma once

#include <string>

#ifdef USE_GHC_FILESYSTEM
#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

#ifdef _WIN32
typedef std::wstring path_string;
#else
typedef std::string path_string;
#endif

fs::path GetExecutablePath();
fs::path GetDefaultLogPath();
