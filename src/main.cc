#include <orient/app.hpp>

#ifdef _MSC_VER
int wmain(int exe_argc, wchar_t** exe_argv) noexcept
#else
int main(int exe_argc, char** exe_argv) noexcept
#endif
{
    return orie::app::main(exe_argc, exe_argv);
}
