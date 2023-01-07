#pragma once
#include "./node.hpp"
#include "./excepts.hpp"

#include <cassert>
#include <vector>

namespace orie {
namespace pred_tree {

enum class node_hint {
    NOT_FOUND, BRIDGE, 
    MODIFIER, PRED
};

template <class iter_t, class sv_t>
class builder {
public:
    using node_type = node<iter_t, sv_t>;
    using char_type = typename sv_t::value_type;
    using strview_t = sv_t;

private:
    static constexpr char_type _cmd_cstr[9][9] = {
        {'-', 't', 'r', 'u', 'e'},            // -true 0
        {'-', 'f', 'a', 'l', 's', 'e'},       // -false 1
        {'-', 'n', 'o', 't'},     {'!'},      // -not 2 3
        {'-', 'a', 'n', 'd'},     {'-', 'a'}, // -and 4 5
        {'-', 'o', 'r'},          {'-', 'o'}, // -or 6 7
        {'-', 't', 'e', 's', 't', 'u', 's', 'e'} // -testuse 8
    };
    static constexpr sv_t cmd_sv[9] = {
        _cmd_cstr[0], _cmd_cstr[1], _cmd_cstr[2],
        _cmd_cstr[3], _cmd_cstr[4], _cmd_cstr[5],
        _cmd_cstr[6], _cmd_cstr[7], _cmd_cstr[8] 
    };

protected:
    static constexpr char_type _lpar = '(', _rpar = ')',
           _lpar_cstr[2] = {'('}, _rpar_cstr[2] = {')'};

    typedef std::pair<node_type*, uint64_t> _brid_pair;
    typedef std::pair<node_type*, uint32_t> _modi_pair;
    uint32_t parentheses_level() const noexcept { return _par_lvl; }

    virtual node_type* pred_dispatch(sv_t cmd) const;
    virtual node_type* modifier_dispatch(sv_t cmd) const;
    virtual _brid_pair bridge_dispatch(sv_t cmd) const;

public:
    node_type* build(sv_t content);
    node_type* build(int exe_argc, const char_type* const* exe_argv);
    void set_bridge_fallback(sv_t f = sv_t()) noexcept { _bridge_fallback = f; }

    node_type* get() const noexcept {
        assert(_pred_stack.empty() || _pred_stack.size() == 1);
        return _pred_stack.empty() ? nullptr : _pred_stack.at(0);
    }
    node_type* release() noexcept; 
    virtual void clear() noexcept; 

    builder() noexcept = default;
    ~builder() noexcept { clear(); }
    builder(const builder<iter_t, sv_t>&) = delete;
    builder<iter_t, sv_t>& operator=(const builder<iter_t, sv_t>&) = delete;
    builder(builder<iter_t, sv_t>&&) = default;
    builder<iter_t, sv_t>& operator=(builder<iter_t, sv_t>&&) = default;

// Since there must be a bridge between two preds, the equality always holds:
// _bridge_stack().size() == _pred_stack().size() - static_cast<size_t>(_next_is_bridge)
// It will be asserted multiple times
private:
    bool _next_is_bridge = false;
    // For empty parentheses detection
    bool _last_is_par_enter = false;
    uint32_t _par_lvl = 0;
    node_type* _adding = nullptr;
    node_hint _adding_hint = node_hint::NOT_FOUND;
    uint64_t _adding_brid_prio = 0;

    sv_t _pred_fallback, _bridge_fallback = cmd_sv[4];

    std::vector<node_type*> _pred_stack;
    std::vector<_modi_pair> _mod_stack;
    std::vector<_brid_pair> _bridge_stack;

    static constexpr size_t _at_builtin_cmds(sv_t sv, size_t from) {
        for (size_t i = from; i < 9; i++)
            if (sv == cmd_sv[i])
                return i;
        return ~size_t();
    }

    // Merge 2 preds and 1 bridge into 1 larger predicate
    void _merge_last_two(); 

    // Apply all modifiers in current parentheses level to "to_apply"
    node_type* _apply_mods(node_type* to_apply); 

    // Called when encountering "("
    void _par_enter();
    // Called when encountering ")"
    void _par_exit();

    // Place the fallback bridge if needed
    // Only returns false if a fallback needs to be placed, but the fallback
    // set via "set_bridge_fallback" is invalid(or not set)
    bool _place_bridge_fallback();
    void _place_adding(sv_t param);
    void _consume_one_param(sv_t param);
};

template <class iter_t, class sv_t>
typename builder<iter_t, sv_t>::node_type*
builder<iter_t, sv_t>::build(sv_t content) {
    clear();
    char_type token_buf[256];
    sv_t param;

    while (!content.empty()) {
        auto [read_sz, tok_sz] = orie::next_token(
            content.data(), content.size(), token_buf, 256
        );
        param = sv_t(token_buf, tok_sz);
        _consume_one_param(param);
        content.remove_prefix(read_sz);
    }

    _place_adding(param);
    if (_par_lvl > 0) {
        // ERROR: ... ( ( ... )
        throw parentheses_mismatch(true);
    } else if (!_next_is_bridge) {
        // ERROR: ... -a ERROR: Empty expression
        char_type the_end[8] = {'t','h','e',' ','e','n','d', '\0'};
        throw missing_predicate(sv_t(the_end), true);
    }

    while (!_bridge_stack.empty())
        _merge_last_two();
    assert(_pred_stack.size() == 1);
    return _pred_stack[0];
}

template <class iter_t, class sv_t>
typename builder<iter_t, sv_t>::node_type*
builder<iter_t, sv_t>::build(int exe_argc, const char_type* const* exe_argv) {
    clear();
    for (int i = 1; i < exe_argc; ++i) 
        _consume_one_param(sv_t(exe_argv[i]));

    _place_adding(exe_argv[exe_argc - 1]);
    if (_par_lvl > 0) {
        // ERROR: ... ( ( ... )
        throw parentheses_mismatch(true);
    } else if (!_next_is_bridge) {
        // ERROR: ... -a ERROR: Empty expression
        char_type the_end[8] = {'t','h','e',' ','e','n','d','\0'};
        throw missing_predicate(sv_t(the_end), true);
    }

    while (!_bridge_stack.empty())
        _merge_last_two();
    assert(_pred_stack.size() == 1);
    return _pred_stack[0];
}

template <class iter_t, class sv_t>
typename builder<iter_t, sv_t>::node_type* 
builder<iter_t, sv_t>::pred_dispatch(sv_t cmd) const {
    switch (_at_builtin_cmds(cmd, 0)) {
        case 0: // -true
            return new truefalse_node<iter_t, sv_t>(true);
        case 1: // -false
            return new truefalse_node<iter_t, sv_t>(false);
        case 8:
            return new __nextparam_tester<iter_t, sv_t>();
    }
    return nullptr;
}

template <class iter_t, class sv_t>
typename builder<iter_t, sv_t>::node_type* 
builder<iter_t, sv_t>::modifier_dispatch(sv_t cmd) const {
    switch (_at_builtin_cmds(cmd, 0)) {
        case 2: case 3: // -not !
            return new not_node<iter_t, sv_t>();
    }
    return nullptr;
}

template <class iter_t, class sv_t>
typename builder<iter_t, sv_t>::node_type* 
builder<iter_t, sv_t>::release() noexcept {
    assert(_pred_stack.empty() || _pred_stack.size() == 1);
    if (_pred_stack.empty())
        return nullptr;
    node_type* ret = _pred_stack.at(0);
    _pred_stack.clear();
    return ret;
}

template <class iter_t, class sv_t>
typename builder<iter_t, sv_t>::_brid_pair
builder<iter_t, sv_t>::bridge_dispatch(sv_t cmd) const {
    switch (_at_builtin_cmds(cmd, 0)) {
        case 4: case 5: // -and -a
            return std::make_pair(new cond_node<iter_t, sv_t>(CONDS::AND), 20);
        case 6: case 7: // -or -o
            return std::make_pair(new cond_node<iter_t, sv_t>(CONDS::OR), 10);
    }
    return std::make_pair(nullptr, 0);
}

template <class iter_t, class sv_t>
void builder<iter_t, sv_t>::clear() noexcept {
    for (node_type* _ : _pred_stack) delete _;
    for (auto& _ : _bridge_stack) delete _.first;
    for (auto& _ : _mod_stack) delete _.first;
    _pred_stack.clear();
    _bridge_stack.clear();
    _mod_stack.clear();
    _par_lvl = 0;
    delete _adding; _adding = nullptr;
    _next_is_bridge = false;
    _last_is_par_enter = false;
}

// Implementation Details of Pred-tree construction from strings

template <class iter_t, class sv_t>
void builder<iter_t, sv_t>::_merge_last_two() {
    // Extract last 2 from predicates and 1 from bridges
    node_type* rhs = _pred_stack.back();
    _pred_stack.pop_back();
    node_type* lhs = _pred_stack.back(),
        *res = _bridge_stack.back().first;
    res->setprev(lhs, true); // Construct the new node
    res->setprev(rhs, false);
    _bridge_stack.pop_back(); // Pop the pointer to bridge
    _pred_stack.back() = res; // Write the new node back, overriding `lhs`
}

template <class iter_t, class sv_t>
typename builder<iter_t, sv_t>::node_type*
builder<iter_t, sv_t>::_apply_mods(node_type* to_apply) {
    while (!_mod_stack.empty() && _mod_stack.back().second == _par_lvl) {
        _mod_stack.back().first->setprev(to_apply, false);
        to_apply = _mod_stack.back().first;
        _mod_stack.pop_back();
    }
    return to_apply;
}

template <class iter_t, class sv_t>
bool builder<iter_t, sv_t>::_place_bridge_fallback() {
    if (!_next_is_bridge)
        return true;

    auto [to_add, prio] = bridge_dispatch(_bridge_fallback);
    if (to_add == nullptr) 
        return false;

    prio += static_cast<uint64_t>(_par_lvl) << 32;
    while (!_bridge_stack.empty() && _bridge_stack.back().second >= prio)
        _merge_last_two();
    _bridge_stack.emplace_back(to_add, prio);
    _next_is_bridge = false;
    return true;
}

template <class iter_t, class sv_t>
void builder<iter_t, sv_t>::_par_enter() { 
    // A parentheses pair must supercede a bridge
    // Ex: ... -a ( ... ) ... Ex2: ( ... ) ...
    if (!_place_bridge_fallback()) 
        throw missing_bridge(sv_t(_lpar_cstr));
    _last_is_par_enter = true;
    ++_par_lvl; 
}

template <class iter_t, class sv_t>
void builder<iter_t, sv_t>::_par_exit() {
    --_par_lvl;
    // ERROR: ... ( ... ) ) ...
    if (_par_lvl == ~uint32_t()) // -1
        throw orie::pred_tree::parentheses_mismatch(false);
    // ERROR empty pair: ... ( ) ...
    if (_last_is_par_enter)
        throw orie::pred_tree::empty_parentheses();
    // ERROR: ... ( ... -a ) ... ERROR: ... ( ... -a ! ) ...
    // ERROR(bridge fallback implicitly added): ... ( ... ! ) ...
    if (!_next_is_bridge)
        throw orie::pred_tree::missing_predicate(sv_t(_rpar_cstr), true);

    // then merge all nodes inside into one large node 
    while (!_bridge_stack.empty() && (_bridge_stack.back().second >> 32) > _par_lvl)
        _merge_last_two();
    // finally handle modifiers before the parentheses
    // -not ( ... )
    _pred_stack.back() = _apply_mods(_pred_stack.back());
}

template <class iter_t, class sv_t>
void builder<iter_t, sv_t>::_place_adding(sv_t param) {
    if (_adding == nullptr)
        return;
    _last_is_par_enter = false;

    switch (_adding_hint) {
    case node_hint::MODIFIER:
        if (!_place_bridge_fallback()) 
            throw missing_bridge(param);
        _mod_stack.emplace_back(_adding, _par_lvl);
        break;

    case node_hint::PRED:
        if (!_place_bridge_fallback())
            throw missing_bridge(param);
        _pred_stack.push_back(_apply_mods(_adding));
        _next_is_bridge = true;
        break;

    case node_hint::BRIDGE:
        // ERROR: ... -a -o ...
        if (!_next_is_bridge) 
            throw missing_predicate(param, true);
    
        while (!_bridge_stack.empty() && _bridge_stack.back().second >= _adding_brid_prio)
            _merge_last_two();
        _bridge_stack.emplace_back(_adding, _adding_brid_prio);
        _next_is_bridge = false;
        break;
    default: std::terminate();
    }

    _adding = nullptr;
}

template <class iter_t, class sv_t>
void builder<iter_t, sv_t>::_consume_one_param(sv_t param) {
    // previous node accepts the parameter `param`
    if (_adding && _adding->next_param(param))
        return; 
    // The previous node exists, but accepts no more parameters. 
    // Push the fully constructed node into stacks, then create a new one with param
    else if (_adding)
        _place_adding(param); // `_adding` became nullptr after being placed
    // `_adding` didn't accept `param`, therefore `param` must be treated as a new node

    assert(_adding == nullptr);
    if (param.size() == 1 && param[0] == '(')
        return _par_enter();
    else if (param.size() == 1 && param[0] == ')')
        return _par_exit();

    // Create a new one with "param"
    else if ((_adding = pred_dispatch(param)) != nullptr) {
        _adding_hint = node_hint::PRED;
    } else if ((_adding = modifier_dispatch(param)) != nullptr) {
        _adding_hint = node_hint::MODIFIER;
    } else if (std::tie(_adding, _adding_brid_prio) = bridge_dispatch(param); _adding) {
        _adding_brid_prio += static_cast<uint64_t>(_par_lvl) << 32;
        _adding_hint = node_hint::BRIDGE;
    } 

    // Got bullsh*t when expecting a node name.
    // Now attempt to place fallback pred, with what we got as its first parameter
    else if ((_adding = pred_dispatch(_pred_fallback)) != nullptr &&
              _adding->next_param(param)) {
        _adding_hint = node_hint::PRED;
    } else 
        // No fallback or fallback does not accept "param"
        throw unknown_node_name(param);
}

}
}
