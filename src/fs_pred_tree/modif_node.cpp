#include <orient/fs_pred_tree/fs_nodes.hpp>

namespace orie {
namespace pred_tree {

tribool_bad downdir_node::apply(fs_data_iter& it) {
    return tribool_bad::Uncertain;
}

bool downdir_node::apply_blocked(fs_data_iter& it) {
    return false;
}

bool downdir_node::next_param(sv_t param) {
    return false;
}

tribool_bad updir_node::apply(fs_data_iter& it) {
    return tribool_bad::Uncertain;
}

bool updir_node::apply_blocked(fs_data_iter& it) {
    return false;
}

}
}
