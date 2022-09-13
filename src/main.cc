#include <orient/app.hpp>

#ifdef _MSC_VER
int wmain(int exe_argc, wchar_t** exe_argv) noexcept
#else
int main(int exe_argc, char** exe_argv) noexcept
#endif
{
    // No C-style buffered write to stdout in all of the code
    std::ios_base::sync_with_stdio(false);
    return orie::app::main(exe_argc, exe_argv);
}
