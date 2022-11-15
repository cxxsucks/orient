#include <orient/fs_pred_tree/fs_nodes.hpp>
#include <algorithm>
#ifndef _WIN32
extern "C" {
#include <sys/wait.h>
}
#endif

namespace orie {
namespace pred_tree {

tribool_bad downdir_node::apply(fs_data_iter& it) {
    if (!prev)
        return tribool_bad::False;
    fs_data_iter down = it.current_dir_iter();
    size_t cnt_true = 0;
    bool has_uncertain = false;

    while (down != down.end()) {
        tribool_bad prev_res = prev->apply_blocked(down);
        if (prev_res.is_uncertain()) 
            has_uncertain = true;
        else if (prev_res == tribool_bad::True) {
            ++cnt_true;
            if (cnt_true > _min_cnt && _max_cnt == 0)
            // No maximum limit and minimum limit reached.
                return tribool_bad::True;
            else if (_max_cnt != 0 && cnt_true > _max_cnt)
            // Maximum match limit exceeded.
                return tribool_bad::False;
        }
        ++down;
    }

    if (cnt_true > _min_cnt)
        return tribool_bad::True;
    return has_uncertain ? tribool_bad::Uncertain : tribool_bad::False;
}

bool downdir_node::apply_blocked(fs_data_iter& it) {
    if (!prev)
        return false;
    fs_data_iter down = it.current_dir_iter();
    // return min < std::count_if(down, down.end, prev->apply_blocked) < max
    // Unfortunately std::count_if will not stop prematurely
    // even if the result is determined.
    size_t cnt_true = 0;

    while (down != down.end()) {
        if (prev->apply_blocked(down)) {
            ++cnt_true;
            if (cnt_true > _min_cnt && _max_cnt == 0)
            // No maximum limit and minimum limit reached.
                return true;
            else if (_max_cnt != 0 && cnt_true > _max_cnt)
            // Maximum match limit exceeded.
                return false;
        }
        ++down;
    }

    // Max limit exceeded is handled inside the loop.
    return cnt_true > _min_cnt;
}

bool downdir_node::next_param(sv_t param) {
    if (param.empty())
        throw invalid_param_name(NATIVE_SV("null parameter"), 
                                 NATIVE_SV("-num"));

    char_t flag = param.front();
    if (flag == '+' || flag == '-')
        param.remove_prefix(1);
    size_t parsed;
    const char_t *__beg = param.data(),
                 *__end = __beg + param.size(),
                 *__numend = orie::from_char_t(__beg, __end, parsed);
    if (__end != __numend)
        return false; // Treat non-number as predicates following it
    if (parsed == 0) {
        NATIVE_STDERR << NATIVE_PATH("Specifying 0 has no effect. By default"
            "-downdir uses +0 as default parameter. To find directories whose "
            "children has no match, use `! -downdir PREDS`.\n");
        return true;
    }

    if (flag == '+') _min_cnt = parsed;
    else if (flag == '-') _max_cnt = parsed;
    else {
        _min_cnt = parsed - 1;
        _max_cnt = parsed + 1;
    }
    return true;
}

bool updir_node::apply_blocked(fs_data_iter& it) {
    fs_data_record up_rec = it.record(1);
    for (auto& done_item : _last_done_q)
        if (up_rec == done_item.first)
            return done_item.second;
    if (!prev || up_rec.file_type() == unknown_tag)
        return false;

    fs_data_iter up_iter = it;
    up_iter.updir();
    bool ret = prev->apply_blocked(up_iter);
    
    std::lock_guard<std::mutex> _lck(_last_done_mut);
    _last_done_q[_last_idx] = std::make_pair(up_rec, ret);
    ++_last_idx; _last_idx &= 7;
    return ret;
}

std::mutex print_node::_out_mut = std::mutex();
print_node::print_node(bool std_out, char_t split) 
    : _format(str_t(NATIVE_PATH("%p")) + split) 
{
    if (std_out)
        _ofs.reset(&NATIVE_STDOUT, [](auto*){});
}

print_node::print_node(bool std_out) {
    if (std_out)
        _ofs.reset(&NATIVE_STDOUT, [](auto*){});
}

bool print_node::apply_blocked(fs_data_iter& it) {
    if (_ofs == nullptr)
        throw uninitialized_node(NATIVE_SV("-fprint missing filename"));
    if (_format.empty())
        throw uninitialized_node(NATIVE_SV("-printf missing format"));
    // TODO: -printf and -fprintf
    std::lock_guard __lck(_out_mut);
    *_ofs << it.path() << _format.back();
    return _ofs->good();
}

bool print_node::next_param(sv_t param) {
    if (_ofs == nullptr) {
#if !defined(_WIN32) || defined(_MSC_VER)
        _ofs.reset(new std::basic_ofstream<char_t>(str_t(param)));
#else
        _ofs.reset(new std::basic_ofstream<char_t>(orie::xxstrcpy(param)));
#endif
        if (!_ofs->good()) {
            _ofs.reset();
#ifndef _WIN32
            throw std::runtime_error(NATIVE_PATH("Open failed: ") + str_t(param));
#else
            throw std::runtime_error(orie::xxstrcpy(sv_t(
                NATIVE_PATH("Open failed: ") + str_t(param)
            )));
#endif
        }
        return true;
    }
    if (_format.empty()) {
        _format = param;
        return true;
    }
    return false;
}

bool del_node::apply_blocked(fs_data_iter& it) {
    // When iterator has left the directory whose name stored at the stack top,
    // all its contents are iterated, and rmdir(2) is safe to be called on it.
    while (!_todel_dirs_stack.empty() && (
        // !it.path().starts_with(_todel_dirs_stack.back()))
           it.path().size() < _todel_dirs_stack.back().size() ||
           ::memcmp(it.path().data(), _todel_dirs_stack.back().data(), 
                    _todel_dirs_stack.back().size()) != 0))
    {
        if (_dry_run)
            NATIVE_STDOUT << NATIVE_PATH("DELETE: ") 
                          << _todel_dirs_stack.back()
                          << NATIVE_PATH('\n');
        else
#ifdef _WIN32
            if (!::RemoveDirectoryW(_todel_dirs_stack.back().c_str()))
                std::wcerr << L"Cannot Delete: " << _todel_dirs_stack.back() 
                           << L" Error " << GetLastError() << L'\n';
#else
            ::rmdir(_todel_dirs_stack.back().c_str());
#endif
        _todel_dirs_stack.pop_back();
    }

    if (it.file_type() == category_tag::dir_tag) {
        _todel_dirs_stack.push_back(it.path());
        return true;
    }

    // Is not a directory, unlink it.
    bool ret = true;
    if (_dry_run)
        NATIVE_STDOUT << NATIVE_PATH("DELETE: ") << it.path()
                      << NATIVE_PATH('\n');
    else 
#ifdef _WIN32
        ret = 0 != ::DeleteFileW(it.path().c_str());
#else
        ret = 0 == ::unlink(it.path().c_str());
#endif

    return ret;
}

bool del_node::next_param(sv_t param) {
    if (param == NATIVE_SV("--dryrun")) {
        _dry_run = true;
        return true;
    }
    return false;
}

del_node::~del_node() noexcept {
    for (const str_t& path : _todel_dirs_stack) 
#ifdef _WIN32
        ::RemoveDirectoryW(path.c_str());
#else
        ::rmdir(path.c_str());
#endif
}

// This implementation sacrificed performance (when constructing cmdline)
// for clarity and better concurrency, as an entire copy of command line
// is allocated and copied.
bool exec_node::apply_blocked(fs_data_iter& it) {
    if (!_parse_finished)
        throw uninitialized_node(NATIVE_SV("--exec"));
    std::vector<str_t> argvstr_to_exec;
{
    std::lock_guard __lk(_names_mut);
    if (it != it.end()) {
        _names_to_pass.push_back(it.path());
        _name_len_left -= it.path().size();
    }
    if (_name_len_left > 0)
        return true;

    argvstr_to_exec.reserve(_exec_cmds.size() + _names_to_pass.size());
    for (const str_t& cmdarg : _exec_cmds) {
        if (cmdarg.find(NATIVE_PATH("{}")) == sv_t::npos) {
            argvstr_to_exec.push_back(cmdarg);
            continue;
        }

        // Substitute the "{}" with all filenames ready to pass.
        for (const str_t& filename : _names_to_pass) {
            str_t replaced = cmdarg;
            size_t pos;
            while ((pos = replaced.find(NATIVE_PATH("{}"))) != str_t::npos)
                replaced.replace(pos, 2, filename);
            // There may be multiple "{}"s
            argvstr_to_exec.push_back(replaced);
        }
    }
    // Cleanup; ready to fork and exec
    _name_len_left = _name_min_len;
    _names_to_pass.clear();
} // Mutex of original name vector unlocked since they are no longer used

    // Confirm from stdin if running `-ok -okdir`
    if (_stdin_confirm) {
        NATIVE_STDERR << NATIVE_PATH("< ")
            << argvstr_to_exec.front() << NATIVE_PATH(" ... ")
            << (it == it.end() ? NATIVE_PATH("") : it.path())
            << NATIVE_PATH(" > ? ");
        char c = std::cin ? std::cin.get() : 'N';
        while (std::cin && std::cin.get() != '\n')
            ;
        if (::toupper(c) != 'Y')
            return false;
    }

#ifndef _WIN32 // Unix fork-exec
    // Construct the pointer to cstrings to pass into `execvp`
    std::unique_ptr<const char*[]> argv_to_exec(
        new const char*[argvstr_to_exec.size() + 1]
    );
    std::transform(argvstr_to_exec.begin(), argvstr_to_exec.end(),
        argv_to_exec.get(), [](const str_t& s) { return s.c_str(); });
    // The last element of argv_to_exec must be nullptr
    argv_to_exec[argvstr_to_exec.size()] = nullptr;

    // Handle `execvp` failure in child process with pipes
    int pipe_fds[2]; 
    if (0 != ::pipe(pipe_fds)) {
        ::perror("pipe");
        return false;
    }
    if (0 != ::fcntl(pipe_fds[1], F_SETFD, 
                     ::fcntl(pipe_fds[1], F_GETFD) | FD_CLOEXEC)) {
        ::perror("fcntl");
        return false;
    }

    // Actual fork-and-exec
    pid_t child = ::fork();
    switch (child) {
    case -1:
        ::perror("fork");
        return false;
    case 0:    
        ::close(pipe_fds[0]); // Child only writes error msg
        if (_stdin_confirm) 
            // TODO: Error checking
            ::dup2(::open("/dev/null", O_RDONLY), 0); 
        if (_from_subdir && it != it.end())
            if (0 != ::chdir(it.parent_path().c_str())) {
                ::perror("chdir");
                return false;
            }

        // Why is `argv` not const in `exec**`?
        ::execvp(argv_to_exec[0], const_cast<char*const*>(argv_to_exec.get()));
        if (-1 == ::write(pipe_fds[1], &errno, sizeof(int))) {
            ::perror("write");
            ::_exit(1);
        }
        ::_exit(0);

    default: {
        ::close(pipe_fds[1]);
        int count, err;
        while ((count = ::read(pipe_fds[0], &err, sizeof(errno))) != -1)
            if (errno != EAGAIN && errno != EINTR)
                break;
        if (count != 0) {
            // Child wrote to err. 
            NATIVE_STDERR << argv_to_exec[0] << ": " << strerror(err) << '\n';
            return false;
        }
        // Waiting for child
        while (::waitpid(child, &err, 0) == -1)
            if (errno != EINTR) {
                ::perror("waitpid");
                return false;
            }
        return _name_min_len > 0 || WEXITSTATUS(err) == 0;
    }
    }
    std::terminate(); // Unreachable

#else // Win32 CreateProcessW (no fork so cannot use _execvp)
    std::wstring cmdline_str;
    // FIXME: MSDN says CreateProcessW modifies cmdline without saying how,
    // so we reserve a very large string for the function's pleasure.
    cmdline_str.reserve(4096);
    // FIXME: CreateProcessW for some reason accepts all cmdline as one string.
    // All cmdline in `argvstr_to_exec` containing whilespaces will break, 
    // even if they were quoted.
    for (std::wstring& arg : argvstr_to_exec)
        (cmdline_str += std::move(arg)).push_back(L' ');

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = ::GetStdHandle(STD_ERROR_HANDLE);
    // Redirect "nul" to stdin if `_stdin_confirm` is set
    if (_stdin_confirm) {
        si.hStdInput = ::CreateFileW(
            L"nul", GENERIC_READ, FILE_SHARE_READ, nullptr,
            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    } 
    else si.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);

    cmdline_str.reserve(cmdline_str.size() + 100);
    DWORD exit_code;
    bool ret = ::CreateProcessW(nullptr, cmdline_str.data(), nullptr, nullptr, true,
                                NORMAL_PRIORITY_CLASS, nullptr, nullptr, &si, &pi)
            && ::WaitForSingleObject(pi.hProcess, INFINITE) 
            && ::GetExitCodeProcess(pi.hProcess, &exit_code) 
            && exit_code == 0;
    ::CloseHandle(pi.hProcess);
    ::CloseHandle(pi.hThread);
    if (_stdin_confirm)
        ::CloseHandle(si.hStdInput);
    return ret;
#endif
}

bool exec_node::next_param(sv_t param) {
    if (_parse_finished)
        return false;
    else if (param == NATIVE_SV(";")) {
        if (_exec_cmds.empty())
            throw invalid_param_name(param, NATIVE_SV("--exec"));
        _parse_finished = true;
        _name_min_len = _name_len_left = -1;
        return true;
    } else if (param == NATIVE_SV("+")) {
        if (_exec_cmds.empty())
            throw invalid_param_name(param, NATIVE_SV("--exec"));
        _parse_finished = true;
        _name_min_len = _name_len_left = 16384;
        return true;
    }

    _exec_cmds.emplace_back(param);
    return true;
}

exec_node::~exec_node() {
    if (_names_to_pass.empty())
        return;
    _name_len_left = -1;
    auto dummy = fs_data_iter::end();
    apply_blocked(dummy);
}

// copy ctors and `operator=`s
exec_node::exec_node(const exec_node&r)
    : _exec_cmds(r._exec_cmds)
    , _names_to_pass(r._names_to_pass)
    , _name_min_len(r._name_min_len)
    , _name_len_left(r._name_len_left)
    , _parse_finished(r._parse_finished)
    , _stdin_confirm(r._stdin_confirm)
    , _from_subdir(r._from_subdir) {}

exec_node& exec_node::operator=(const exec_node& r) {
    if (this != &r) {
        this->~exec_node();
        new (this) exec_node(r);
    }
    return *this;
}

/* print_node& print_node::operator=(const print_node& r) {
    if (this != &r) {
        this->~print_node();
        new (this) print_node(r);
    }
    return *this;
} */

updir_node::updir_node(const updir_node &r)
    : _last_done_q(r._last_done_q), _last_idx(r._last_idx) {}
updir_node &updir_node::operator=(const updir_node &r) {
    if (this != &r) {
        this->~updir_node();
        new (this) updir_node(r);
    }
    return *this;
}
}
}
