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
    static constexpr char_type cmd_cstr[9][9] = {
        {'-', 't', 'r', 'u', 'e'},            // -true 0
        {'-', 'f', 'a', 'l', 's', 'e'},       // -false 1
        {'-', 'n', 'o', 't'},     {'!'},      // -not 2 3
        {'-', 'a', 'n', 'd'},     {'-', 'a'}, // -and 4 5
        {'-', 'o', 'r'},          {'-', 'o'}, // -or 6 7
        {'-', 't', 'e', 's', 't', 'u', 's', 'e'} // -testuse 8
    };
    static constexpr sv_t cmd_sv[9] = {
        cmd_cstr[0], cmd_cstr[1], cmd_cstr[2],
        cmd_cstr[3], cmd_cstr[4], cmd_cstr[5],
        cmd_cstr[6], cmd_cstr[7], cmd_cstr[8] 
    };

protected:
    static constexpr char_type lpar = '(', rpar = ')',
           lpar_cstr[2] = {'('}, rpar_cstr[2] = {')'};

    typedef std::pair<node_type*, uint64_t> _brid_pair;
    typedef std::pair<node_type*, uint32_t> _modi_pair;
    uint32_t parentheses_level() const noexcept { return par_lvl; }

    virtual node_type* pred_dispatch(sv_t cmd) const;
    virtual node_type* modifier_dispatch(sv_t cmd) const;
    virtual _brid_pair bridge_dispatch(sv_t cmd) const;

public:
    node_type* build(sv_t content);
    node_type* build(int exe_argc, const char_type* const* exe_argv);
    void set_bridge_fallback(sv_t f = sv_t()) noexcept { bridge_fallback = f; }

    node_type* get() const noexcept {
        assert(pred_stack.empty() || pred_stack.size() == 1);
        return pred_stack.empty() ? nullptr : pred_stack.at(0);
    }
    node_type* release() noexcept {
        assert(pred_stack.empty() || pred_stack.size() == 1);
        if (pred_stack.empty())
            return nullptr;
        node_type* ret = pred_stack.at(0);
        pred_stack.clear();
        return ret;
    }

    virtual void clear() noexcept {
        for (node_type* _ : pred_stack) delete _;
        for (auto& _ : bridge_stack) delete _.first;
        for (auto& _ : mod_stack) delete _.first;
        pred_stack.clear();
        bridge_stack.clear();
        mod_stack.clear();
        par_lvl = 0;
        delete adding; adding = nullptr;
        next_is_bridge = false;
    }

    builder() noexcept = default;
	~builder() noexcept { clear(); }
    builder(const builder<iter_t, sv_t>&) = delete;
    builder<iter_t, sv_t>& operator=(const builder<iter_t, sv_t>&) = delete;
    builder(builder<iter_t, sv_t>&&) = default;
    builder<iter_t, sv_t>& operator=(builder<iter_t, sv_t>&&) = default;

// Since there must be a bridge between two preds, the equality always holds:
// bridge_stack().size() == pred_stack().size() - static_cast<size_t>(next_is_bridge)
// It will be asserted multiple times
private:
    bool next_is_bridge = false;
    uint32_t par_lvl = 0;
    node_type* adding = nullptr;
    node_hint adding_hint;
    uint64_t adding_brid_prio;

    sv_t pred_fallback, bridge_fallback = cmd_sv[4];

    std::vector<node_type*> pred_stack;
    std::vector<_modi_pair> mod_stack;
    std::vector<_brid_pair> bridge_stack;

    static constexpr size_t _at_builtin_cmds(sv_t sv, size_t from) {
        for (size_t i = from; i < 9; i++)
            if (sv == cmd_sv[i])
                return i;
        return ~size_t();
    }

    // Implementation Details of Pred-tree construction from strings

    // Merge 2 preds and 1 bridge into 1 larger predicate
    void _merge_last_two() {
        // Extract last 2 from predicates and 1 from bridges
        node_type* rhs = pred_stack.back();
        pred_stack.pop_back();
        node_type* lhs = pred_stack.back(),
            *res = bridge_stack.back().first;
        res->setprev(lhs, true); // Construct the new node
        res->setprev(rhs, false);
        bridge_stack.pop_back(); // Pop the pointer to bridge
        pred_stack.back() = res; // Write the new node back, overriding `lhs`
    }

    // Apply all modifiers in current parentheses level to "to_apply"
    node_type* _apply_mods(node_type* to_apply) {
        while (!mod_stack.empty() && mod_stack.back().second == par_lvl) {
            mod_stack.back().first->setprev(to_apply, false);
            to_apply = mod_stack.back().first;
            mod_stack.pop_back();
        }
        return to_apply;
    }

    // Place the fallback bridge if needed
    // Only returns false if a fallback needs to be placed, but the fallback
    // set via "set_bridge_fallback" is invalid(or not set)
    bool _place_bridge_fallback() {
        if (!next_is_bridge)
            return true;

        auto [to_add, prio] = bridge_dispatch(bridge_fallback);
        if (to_add == nullptr) 
            return false;

        prio += static_cast<uint64_t>(par_lvl) << 32;
        while (!bridge_stack.empty() && bridge_stack.back().second >= prio)
            _merge_last_two();
        bridge_stack.emplace_back(to_add, prio);
        next_is_bridge = false;
        return true;
    }

    // Called when encountering "("
    void _par_enter() { 
        // A parentheses pair must supercede a bridge
        // Ex: ... -a ( ... ) ... Ex2: ( ... ) ...
        if (!_place_bridge_fallback()) 
            throw missing_bridge(sv_t(lpar_cstr));
        ++par_lvl; 
    }

    // Called when encountering ")"
    void _par_exit() {
        --par_lvl;
        // ERROR: ... ( ... ) ) ...
        if (par_lvl == ~uint32_t()) // -1
			throw orie::pred_tree::parentheses_mismatch(false);
        // ERROR empty pair: ... ( ) ...
        if (!bridge_stack.empty() && (bridge_stack.back().second >> 32) <= par_lvl)
			throw orie::pred_tree::empty_parentheses();
        // ERROR: ... ( ... -a ) ... ERROR: ... ( ... -a ! ) ...
        // ERROR(bridge fallback implicitly added): ... ( ... ! ) ...
        if (!next_is_bridge)
			throw orie::pred_tree::missing_predicate(sv_t(rpar_cstr), true);

        // then merge all nodes inside into one large node 
        while (!bridge_stack.empty() && (bridge_stack.back().second >> 32) > par_lvl)
            _merge_last_two();
        // finally handle modifiers before the parentheses
        // -not ( ... )
        pred_stack.back() = _apply_mods(pred_stack.back());
    }

    void _place_adding(sv_t param) {
        if (adding == nullptr)
            return;

        switch (adding_hint) {
        case node_hint::MODIFIER:
            if (!_place_bridge_fallback()) 
                throw missing_bridge(param);
            mod_stack.emplace_back(adding, par_lvl);
            break;

        case node_hint::PRED:
            if (!_place_bridge_fallback())
                throw missing_bridge(param);
            pred_stack.push_back(_apply_mods(adding));
            next_is_bridge = true;
            break;

        case node_hint::BRIDGE:
            // ERROR: ... -a -o ...
            if (!next_is_bridge) 
                throw missing_predicate(param, true);
		
            while (!bridge_stack.empty() && bridge_stack.back().second >= adding_brid_prio)
                _merge_last_two();
            bridge_stack.emplace_back(adding, adding_brid_prio);
            next_is_bridge = false;
            break;
        default: std::terminate();
        }

        adding = nullptr;
    }

    void _consume_one_param(sv_t param) {
        // previous node accepts the parameter `param`
        if (adding && adding->next_param(param))
            return; 
        // The previous node exists, but accepts no more parameters. 
        // Push the fully constructed node into stacks, then create a new one with param
        else if (adding)
            _place_adding(param); // `adding` became nullptr after being placed
        // `adding` didn't accept `param`, therefore `param` must be treated as a new node

        assert(adding == nullptr);
        if (param.size() == 1 && param[0] == '(')
            return _par_enter();
        else if (param.size() == 1 && param[0] == ')')
            return _par_exit();

        // Create a new one with "param"
        else if ((adding = pred_dispatch(param))) {
            adding_hint = node_hint::PRED;
        } else if ((adding = modifier_dispatch(param))) {
            adding_hint = node_hint::MODIFIER;
        } else if (std::tie(adding, adding_brid_prio) = bridge_dispatch(param); adding) {
            adding_brid_prio += static_cast<uint64_t>(par_lvl) << 32;
            adding_hint = node_hint::BRIDGE;
        } 

        // Got bullsh*t when expecting a node name.
        // Now attempt to place fallback pred, with what we got as its first parameter
        else if ((adding = pred_dispatch(pred_fallback)) && adding->next_param(param)) {
            adding_hint = node_hint::PRED;
        } else 
            // No fallback or fallback does not accept "param"
            throw unknown_node_name(param);
    }
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
    if (par_lvl > 0) {
        // ERROR: ... ( ( ... )
        throw parentheses_mismatch(true);
    } else if (!next_is_bridge) {
        // ERROR: ... -a ERROR: Empty expression
        char_type the_end[8] = {'t','h','e',' ','e','n','d'};
        throw missing_predicate(sv_t(the_end), true);
    }

    while (!bridge_stack.empty())
        _merge_last_two();
    assert(pred_stack.size() == 1);
    return pred_stack[0];
}

template <class iter_t, class sv_t>
typename builder<iter_t, sv_t>::node_type*
builder<iter_t, sv_t>::build(int exe_argc, const char_type* const* exe_argv) {
    clear();
    for (int i = 1; i < exe_argc; ++i) 
        _consume_one_param(sv_t(exe_argv[i]));

    _place_adding(exe_argv[exe_argc - 1]);
    if (par_lvl > 0) {
        // ERROR: ... ( ( ... )
        throw parentheses_mismatch(true);
    } else if (!next_is_bridge) {
        // ERROR: ... -a ERROR: Empty expression
        char_type the_end[8] = {'t','h','e',' ','e','n','d'};
        throw missing_predicate(sv_t(the_end), true);
    }

    while (!bridge_stack.empty())
        _merge_last_two();
    assert(pred_stack.size() == 1);
    return pred_stack[0];
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

}
}

