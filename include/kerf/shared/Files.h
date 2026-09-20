#pragma once

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <vector>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace kerf {
namespace Files {

inline std::filesystem::path executableDirectory()
{
#if defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::vector<char> buf(size);
    if (_NSGetExecutablePath(buf.data(), &size) != 0)
    {
        throw std::runtime_error("Failed to get executable path");
    }
    std::filesystem::path exe(buf.data());
#elif defined(_WIN32)
    wchar_t buf[MAX_PATH];
    DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (n == 0 || n == MAX_PATH)
    {
        throw std::runtime_error("Failed to get executable path");
    }
    std::filesystem::path exe(buf);
#else
    std::filesystem::path exe = "/proc/self/exe";
#endif
    return std::filesystem::weakly_canonical(exe).parent_path();
}

inline std::string resolvePath(const std::string& path)
{
    std::filesystem::path p(path);
    if (p.is_relative())
    {
        p = executableDirectory() / p;
    }
    return p.string();
}

inline std::string readTextFile(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file)
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

} // namespace Files
} // namespace kerf
