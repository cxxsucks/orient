#pragma once
#include <orient/pred_tree/builder.hpp>
#include <orient/fs/data_iter.hpp>

namespace orie {
namespace pred_tree {

class fs_expr_builder : public builder<fs_data_iter, sv_t>
{
    using _base_ty = builder<fs_data_iter, sv_t>;
    // Use std::map instead? Or group by first letter?
    static constexpr sv_t cmd_sv[] = {
        NATIVE_SV("-name"),     NATIVE_SV("-iname"),    NATIVE_SV("-lname"),  // 0 1 2
        NATIVE_SV("-path"),     NATIVE_SV("-ipath"),    // 3 4
        NATIVE_SV("-strstr"),   NATIVE_SV("-istrstr"),  // 5 6
        NATIVE_SV("-regex"),    NATIVE_SV("-iregex"),   NATIVE_SV("-bregex"), // 7 8 9
        NATIVE_SV("-ibregex"),  NATIVE_SV("-type"),     // 10 11
        NATIVE_SV("-size"),     NATIVE_SV("-samefile"), NATIVE_SV("-inode"),  // 12 13 14
        NATIVE_SV("-atime"),    NATIVE_SV("-mtime"),    NATIVE_SV("-ctime"),  // 15 16 17
        NATIVE_SV("-amin"),     NATIVE_SV("-mmin"),     NATIVE_SV("-cmin"),   // 18 19 20
        NATIVE_SV("-anewer"),   NATIVE_SV("-mnewer"),   NATIVE_SV("-cnewer"), // 21 22 23
        NATIVE_SV("-uid"),      NATIVE_SV("-gid"),      // 24 25
        NATIVE_SV("-empty"),    NATIVE_SV("-access"),   NATIVE_SV("-perm"),   // 26 27 28
        NATIVE_SV("-readable"), NATIVE_SV("-writable"), NATIVE_SV("-executable"),// 29 30 31
        NATIVE_SV("-user"),     NATIVE_SV("-group"),    NATIVE_SV("-context"),// 32 33 34
        NATIVE_SV("-nogroup"),  NATIVE_SV("-nouser"),   // 35 36
        NATIVE_SV("-content-strstr"),   NATIVE_SV("-content-regex"),          // 37 38
        NATIVE_SV("-updir"),    NATIVE_SV("-downdir"),  // 39 40
        NATIVE_SV("-print"),    NATIVE_SV("-printf"),   NATIVE_SV("-print0"), // 41 42 43
        NATIVE_SV("-fprint"),   NATIVE_SV("-fprintf"),  NATIVE_SV("-fprint0"),// 44 45 46
        NATIVE_SV("-ok"),       NATIVE_SV("-okdir"),    // 47 48
        NATIVE_SV("-exec"),     NATIVE_SV("-execdir"),  // 49 50
        NATIVE_SV("-delete"),   NATIVE_SV("-prune"),    // 51 52
        NATIVE_SV("-ls"),       NATIVE_SV("-fls"),      // 53 54
        NATIVE_SV("-fuzz"),     NATIVE_SV("-content-fuzz"),NATIVE_SV("-quit"),// 55 56 57
        NATIVE_SV("-prunemod"), NATIVE_SV("-quitmod")   // 58 59
    };

    static constexpr size_t _at_builtin_cmds(sv_t sv);

    // The most recently built expression contain action nodes like
    // -print -exec, meaning that "printing result" after matching is
    // not needed. TODO: mutable is too dumb.
    mutable bool _has_action = false,
                 _has_async = false;

protected:
    node_type* pred_dispatch(strview_t cmd) const override;
    node_type* modifier_dispatch(strview_t cmd) const override;

public:
    void clear() noexcept override;
    bool has_action() const noexcept { return _has_action; }
    bool has_async() const noexcept { return _has_async; }

    fs_expr_builder() noexcept = default;
    fs_expr_builder(const fs_expr_builder&) = delete;
    fs_expr_builder(fs_expr_builder&&) = default;
    fs_expr_builder& operator=(const fs_expr_builder&) = delete;
    fs_expr_builder& operator=(fs_expr_builder&&) = default;
};

}
}