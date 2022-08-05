#include <orient/fs_pred_tree/fs_nodes.hpp>

namespace orie {
namespace pred_tree {

bool num_node::apply_blocked(fs_data_iter& it) {
    return false;
}

bool num_node::next_param(sv_t param) {
    return false;
}

bool empty_node::apply_blocked(fs_data_iter& it) {
    return false;
}

bool empty_node::next_param(sv_t param) {
    return false;
}

bool access_node::apply_blocked(fs_data_iter& it) {
    return false;
}

bool access_node::next_param(sv_t param) {
    return false;
}

bool perm_node::apply_blocked(fs_data_iter& it) {
    return false;
}

bool perm_node::next_param(sv_t param) {
    return false;
} 

bool username_node::apply_blocked(fs_data_iter& it) {
    return false;
}

bool username_node::next_param(sv_t param) {
    return false;
}

#ifdef ORIE_NEED_SELINUX
bool selcontext_node::apply_blocked(fs_data_iter& it) {
    return false;
}

bool selcontext_node::next_param(sv_t param) {
    return false;
}
#endif

}
}
