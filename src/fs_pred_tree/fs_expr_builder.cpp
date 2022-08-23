#include <orient/fs_pred_tree/fs_expr_builder.hpp>
#include <orient/fs_pred_tree/fs_nodes.hpp>

namespace orie {
namespace pred_tree {

constexpr size_t fs_expr_builder::_at_builtin_cmds(orie::sv_t cmd) {
    for (size_t i = 0; i < sizeof(cmd_sv) / sizeof(sv_t); i++) 
        if (cmd == cmd_sv[i])
            return i;
    return ~size_t();
}

fs_node *fs_expr_builder::pred_dispatch(orie::sv_t cmd) const {
    if (fs_node* res = _base_ty::pred_dispatch(cmd); res)
        return res;
    auto at = _at_builtin_cmds(cmd);
    if (at >= 41 && at <= 51)
        _has_action = true;

    // A 50-branched switch TAT
    switch (at) {
    // glob_node
    case 0: return new glob_node(false, false, false);
    case 1: return new glob_node(false, false, true);
    case 2: return new glob_node(false, true, false);
    case 3: return new glob_node(true, false, false);
    case 4: return new glob_node(true, false, true);

    //strstr_node
    case 5: return new strstr_node(false, false, false);
    case 6: return new strstr_node(false, false, true);

    //regex_node
    case 7: return new regex_node(true, false, false, false);
    case 8: return new regex_node(true, false, false, true);
    case 9: return new regex_node(false, false, false, false);
    case 10: return new regex_node(false, false, false, true);

    // file stats
    case 11: return new type_node;
    case 12: return new num_node(num_node::stamp::SIZE);
    case 13: case 14: // num_node support compares
        return new num_node(num_node::stamp::INODE);
    case 15: return new num_node(num_node::stamp::ATIME);
    case 16: return new num_node(num_node::stamp::MTIME);
    case 17: return new num_node(num_node::stamp::CTIME);
    case 18: case 21: // [amc]newer compares by minute
        return new num_node(num_node::stamp::AMIN);
    case 19: case 22: 
        return new num_node(num_node::stamp::MMIN);
    case 20: case 23: 
        return new num_node(num_node::stamp::CMIN);
    case 24: return new num_node(num_node::stamp::UID);
    case 25: return new num_node(num_node::stamp::GID);

    case 26: return new empty_node;
    case 27: return new access_node(0);
    case 28: return new perm_node;
    case 29: return new access_node(R_OK);
    case 30: return new access_node(W_OK);
    case 31: return new access_node(X_OK);
    case 32: return new username_node(false);
    case 33: return new username_node(true);
#ifdef ORIE_NEED_SELINUX
    case 34: return new selcontext_node;
#endif
    case 35: return new baduser_node(true);
    case 36: return new baduser_node(false);

    // content
    case 37: 
        _has_async = true;
        return new content_strstr_node;
    case 38: 
        _has_async = true;
        return new content_regex_node;

    // action
    // 41~46 print_node 51 del_node not implemented
    case 47: return new exec_node(true, false);
    case 48: return new exec_node(true, true);
    case 49: return new exec_node(false, false);
    case 50: return new exec_node(false, true);
    case 52: return new prune_node;
    }
    return nullptr;
}

fs_node *fs_expr_builder::modifier_dispatch(orie::sv_t cmd) const {
    if (fs_node* res = _base_ty::modifier_dispatch(cmd); res)
        return res;
    switch (_at_builtin_cmds(cmd)) {
    case 39: return new updir_node;
    case 40: return new downdir_node;
    }
    return nullptr;
}

void fs_expr_builder::clear() noexcept {
    _base_ty::clear();
    _has_action = false;
    _has_async = false;
}

}
}