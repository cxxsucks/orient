#include <orient/app.hpp>

#ifdef _WIN32
#ifdef _MSC_VER
int wmain(int exe_argc, wchar_t** exe_argv) noexcept {
#else // Win32 not MSVC
extern "C" {
#include <shellapi.h>
}
int main() {
    int exe_argc;
    wchar_t** exe_argv = ::CommandLineToArgvW(::GetCommandLineW(), &exe_argc);
#endif
#else // Not Win32
int main(int exe_argc, char** exe_argv) noexcept {
#endif
    // No C-style buffered write to stdout in all of the code
    std::ios_base::sync_with_stdio(false);
    return orie::app::main(exe_argc, exe_argv);
}
