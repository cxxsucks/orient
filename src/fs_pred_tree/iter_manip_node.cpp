#include <orient/fs_pred_tree/fs_nodes.hpp>

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
    it.close_fsdb_view();
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
    const char_t *beg = param.data(),
                 *end = beg + param.size(),
                 *numend = orie::from_char_t(beg, end, parsed);
    if (end != numend)
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
    if (!prev)
        return false;

    fs_data_iter up_iter = it;
    up_iter.updir();
    bool ret = prev->apply_blocked(up_iter);
    
    std::lock_guard<std::mutex> _lck(_last_done_mut);
    _last_done_q[_last_idx] = std::make_pair(up_rec, ret);
    ++_last_idx; _last_idx &= 7;
    return ret;
}

bool prunemod_node::apply_blocked(fs_data_iter& it) {
    if (it.file_type() != orie::dir_tag)
        return true;
    if (prev == nullptr || prev->apply_blocked(it))
        it.disable_pending_recursion();
    return true;
}

}
}
