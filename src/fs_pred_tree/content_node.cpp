#include <orient/fs_pred_tree/fs_nodes.hpp>

namespace orie {
namespace pred_tree {

tribool_bad content_regex_node::apply(fs_data_iter& it) {
    return tribool_bad::Uncertain;
}

bool content_regex_node::apply_blocked(fs_data_iter& it) {
    return false;
}

bool content_regex_node::next_param(sv_t param) {
    return false;
}

tribool_bad content_strstr_node::apply(fs_data_iter& it) {
    return tribool_bad::Uncertain;
}

bool content_strstr_node::apply_blocked(fs_data_iter& it) {
    return false;
}

bool content_strstr_node::next_param(sv_t param) {
    return false;
}


}
}
