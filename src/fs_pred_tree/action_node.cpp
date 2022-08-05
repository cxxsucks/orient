#include <orient/fs_pred_tree/fs_nodes.hpp>

namespace orie {
namespace pred_tree {

bool print_node::apply_blocked(fs_data_iter& it) {
    return false;
}

bool print_node::next_param(sv_t param) {
    return false;
}

bool del_node::apply_blocked(fs_data_iter& it) {
    return false;
}

bool del_node::next_param(sv_t param) {
    return false;
}

bool exec_node::apply_blocked(fs_data_iter& it) {
    return false;
}

bool exec_node::next_param(sv_t param) {
    return false;
}

}
}
