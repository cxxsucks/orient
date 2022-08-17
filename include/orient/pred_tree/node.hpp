#pragma once

#include <memory>
#include <iterator>
#include <ctime>
#include <string_view>

#include "excepts.hpp"

namespace orie {
namespace pred_tree {

struct tribool_bad {
    enum tribool_val : uint8_t { 
        False = 0, True = 1,
        // UncertainFalse = 2, UncertainTrue = 3,
        Uncertain = 2
    };
    tribool_val val;

    constexpr tribool_bad(bool b = false) noexcept
        : val(b ? True : False) {}
    constexpr tribool_bad(tribool_val b) noexcept : val(b) {}
    // constexpr tribool_bad(const tribool_bad &ub) noexcept 
    //     : val(ub.val) {}

    explicit constexpr operator bool() const noexcept {return val == True;}
    constexpr bool operator==(const tribool_bad& rhs) const noexcept {
        return val == rhs.val && !((val | rhs.val) & 2);
    }
    constexpr bool operator!=(const tribool_bad& rhs) const noexcept {
        return val != rhs.val || ((val | rhs.val) & 2);
    }
    constexpr bool is_uncertain() const noexcept {
        return val & 2;
    }

    constexpr tribool_bad& operator^=(const tribool_bad& rhs) noexcept {
        val = static_cast<tribool_val>(((rhs.val | val) & 2) | ((rhs.val ^ val) & 1));
        return *this;
    }
    constexpr tribool_bad& operator|=(const tribool_bad& rhs) noexcept {
        val = (rhs.val == True || val == True) ? True :
            static_cast<tribool_val>(rhs.val | val);
        return *this;
    }
    constexpr tribool_bad& operator&=(const tribool_bad& rhs) noexcept {
        val = (rhs.val == False || val == False) ? False :
            static_cast<tribool_val>(((rhs.val | val) & 2) | (rhs.val & val & 1));
        return *this;
    }

    constexpr tribool_bad operator!() const noexcept {
        tribool_bad tmp(*this);
        tmp.val = static_cast<tribool_val>(tmp.val ^ 1);
        return tmp;
    }
    constexpr tribool_bad operator^(const tribool_bad& rhs) const noexcept {
        return tribool_bad(*this) ^= rhs;
    }
    constexpr tribool_bad operator|(const tribool_bad& rhs) const noexcept {
        return tribool_bad(*this) |= rhs;
    }
    constexpr tribool_bad operator&(const tribool_bad& rhs) const noexcept {
        return tribool_bad(*this) &= rhs;
    }
};

enum class CONDS : int8_t {
    OR, AND, XOR,
    NOR, NAND, XNOR
};

/** @class orie::pred_tree::node
 * @brief A tree-shaped predicate judging whether a iterator satisfies
 * the predicate it describes.
 * @ingroup orient @ingroup pred_tree
 * @tparam iter_t Type of iterator being judged
 * @note This is the Base class and must be subclassed to offer functionality.
 * There are a number of virtual functions which have to be (re)implemented,
 * including whether the node is a leaf node or has child trees.  */
template <class iter_t, class sv_t> class node {
protected:
    node() noexcept = default;

public:
    /** @brief Test whether an iterator satisfies the predicate. 
     * @return Unlike @c "apply", a definite answer (true/false) will be returned
     * however long it may take.
     * @note Pure virtual on base node. */
    virtual bool apply_blocked(iter_t& it) = 0;
    /** @brief Test whether an iterator satifies the predicate.
     * @return May return one of True, False or "Too Complicated, not Done" (uncertain)
     * @retval uncertain Subclasses are free to provide a hint that computation is too heavy and
     * shall be run in a seperate thread.
     * @note Default implementation simply call "apply_blocked" */
    virtual tribool_bad apply(iter_t& it) {
        return static_cast<tribool_bad>(apply_blocked(it));
    }
    /** @brief Some node accept options as strings, which are parsed by passing them here.
     * @param param The next option string to be parsed.
     * @note Multiple option strings must be parsed by multiple calls to this function 
     * in order, one at a time.
     * @return Whether the parameter is accepted. */
    virtual bool next_param(sv_t) { return false; }

    // Return a pointer to a shallow, polymorphic copy of the pred tree in the heap.
    // Return value must be deleted.
    // Implementation MUST NOT call default clone_deep; clone_deep calls clone.
    virtual node<iter_t, sv_t>* clone() const = 0;
    // Return a pointer to a deep, polymorphic copy of the pred tree in the heap. 
    // Return value must be deleted. Default implementation calls clone.
    virtual node<iter_t, sv_t>* clone_deep() const {return clone();}
    
    /** @brief If possible, set @a this node's @p is_left child node to @p heap_node
     * @param is_left true if @p heap_node is to be placed on left child, otherwise right.
     * @param heap_node The child node in heap.
     * @throws std::invalid_argument when trying to give a leaf node a child.
     * @warning @p heap_node MUST be in the @b heap, or auto deletion crashes the app.
     * @note The node being replaced, if any, is auto-deleted.
     * @note If the node only has room for one child, @p is_left is ignored.
     * @note Default implementation of base class just throws. */
    virtual void setprev(orie::pred_tree::node<iter_t, sv_t>*, bool) {
        throw std::invalid_argument("This finder cannot own previous tasks!");
    }

    // Updates the value returned by "cost". Call it on root node after modification.
    // Base implementation does nothing.
    virtual void update_cost() noexcept {}
    // Whether the execution order of left and right child matters.
    // True if the node is a leaf node or has only 1 child node
    virtual bool communicative() const noexcept { return true; }
    // Get the expected execution time.
    virtual double cost() const noexcept = 0;
    // Get the expected success rate.
    virtual double success_rate() const noexcept = 0;

    virtual ~node() noexcept = default;
};

// Handles -true -false. Has no child.
template <class iter_t, class sv_t>
struct truefalse_node : public node<iter_t, sv_t> {
    // Public field: "apply" and "apply_blocked" always return its value.
    bool always;
    truefalse_node(bool _res) noexcept : always(_res) {}

    node<iter_t, sv_t>* clone() const override {
        return new truefalse_node(*this);
    }

    // Instantly return the field "always" for true/false node
    bool apply_blocked(iter_t&) override {return always;}
    // Does nothing for true/false node.
    // void update_cost() noexcept override {}

    // 1.0 if "always" is true, 0.0 otherwise
    double success_rate() const noexcept override {
        return always ? 1.0 : 0.0;
    }
    // "Instant" (1e-8)
    double cost() const noexcept override { return 1e-8; }

    ~truefalse_node() noexcept override = default;
};

/** @class mod_base_node
 * @brief Base node for modifiers, which can have exactly one child.
 * This base class implements some abstract methods taliored for modifiers.
 * @note Modifiers do something before/after delegating the iterator to its
 * child, like @c not inverts the result of its child.  */
template <class iter_t, class sv_t>
class mod_base_node : public node<iter_t, sv_t> {
protected:    
    // Previous node
    std::shared_ptr<node<iter_t, sv_t>> prev;
    // Memorendum of previous results to avoid recalculation in recursion
    double prev_succ, prev_cost;

public:
    // Communicative if its child is communicative. Can be overriden.
    bool communicative() const noexcept override {
        return prev ? prev->communicative() : true;
    }
    // The modifier's cost is its child's cost. This value is memorized.
    // Can be overriden
    double cost() const noexcept override { return prev_cost; }
    double success_rate() const noexcept override { return prev_cost; }

    tribool_bad apply(iter_t& it) override {
        return prev ? prev->apply(it) : false;
    }

    // Update the child's cost, then cache the result (plus some penalty).
    void update_cost() noexcept override {
        if (prev)
            prev->update_cost();
        prev_cost = prev ? prev->cost() + 1e-8 : 1e-8;
        prev_succ = prev ? prev->success_rate() : 0.0;
    }

    // Resets its only child. The boolean parameter is useless. 
    void setprev(node<iter_t, sv_t>* node_on_heap, bool) override {
        prev.reset(node_on_heap);
    }
    node<iter_t, sv_t>* clone_deep() const override {
        mod_base_node<iter_t, sv_t>* res = 
            static_cast<mod_base_node<iter_t, sv_t>*>(this->clone());
        res->prev_succ = prev_succ;
        res->prev_cost = prev_cost;
        res->prev = std::shared_ptr<node<iter_t, sv_t>>(prev->clone_deep());
        return res;
    }

    mod_base_node() noexcept = default;
    ~mod_base_node() noexcept override = default;
};

// The "not" modifier. Inverts its child's result.
template <class iter_t, class sv_t>
struct not_node : public mod_base_node<iter_t, sv_t> {
    bool apply_blocked(iter_t& entry) override {
        return mod_base_node<iter_t, sv_t>::prev ?
            !mod_base_node<iter_t, sv_t>::prev->apply_blocked(entry) : false;
    }
    tribool_bad apply(iter_t& entry) override {
        return mod_base_node<iter_t, sv_t>::prev ?
            !mod_base_node<iter_t, sv_t>::prev->apply(entry) : tribool_bad::False;
    }

    double success_rate() const noexcept override {
        return 1.0 - mod_base_node<iter_t, sv_t>::prev_succ;
    }

    node<iter_t, sv_t>* clone() const override {
        return new not_node(*this);
    }
    not_node() noexcept = default;
    ~not_node() noexcept override = default;
};

/** @class cond_node
 * @brief Implements bridge nodes @c -a, @c -o, among others.
 * Would go for the shortest path, the path that is likely to get a result
 * in the shortest time.
 * @note call @a update_cost before @c apply* to scan and
 * cache the best path */
template <class iter_t, class sv_t>
class cond_node : public node<iter_t, sv_t> {
    std::shared_ptr<node<iter_t, sv_t>> _left, _right;
    CONDS _cond;
    double _lsucc = 0.5, _lcost = 100, _rsucc = 0.5, _rcost = 100;
    bool _r2l = false, _commu = true;

public:
    bool communicative() const noexcept override { return _commu; }

    void update_cost() noexcept override {
        if (_left) {
            _left->update_cost();
            _lsucc = _left->success_rate();
            _lcost = _left->cost() + 1e-8;
        } else _lsucc = 0.0, _lcost = 1e-8;
        if (_right) {
            _right->update_cost();
            _rsucc = _right->success_rate();
            _rcost = _right->cost() + 1e-8;
        } else _rsucc = 0.0, _rcost = 1e-8;

        if (!_right || !_right->communicative() ||
                !_left || !_left->communicative()) {
            _commu = false;
            _r2l = false;
            return;
        }

        switch (_cond) {
        case CONDS::OR: case CONDS::NOR:
            // Right succeeds with less price
            _r2l = _rcost + _lcost * (1 - _rsucc) < _lcost + _rcost * (1 - _lsucc);
            break;
        case CONDS::AND: case CONDS::NAND:
            // Right fails with less price
            _r2l = _rcost + _lcost * _rsucc < _lcost + _rcost * _lsucc;
            break;
        case CONDS::XNOR: case CONDS::XOR: 
            _r2l = false;   // Must evaluate both
        }   
    } 

    double success_rate() const noexcept override {
        switch (_cond) {
        case CONDS::OR:
            return 1.0 - (1.0 - _lsucc) * (1.0 - _rsucc);
        case CONDS::NOR:
            return (1.0 - _lsucc) * (1.0 - _rsucc);
        case CONDS::AND:
            return _lsucc * _rsucc;
        case CONDS::NAND:
            return 1 - _lsucc * _rsucc;
        case CONDS::XOR:
            return _lsucc * (1 - _rsucc) + _rsucc * (1 - _lsucc);
        case CONDS::XNOR:
            return _lsucc * _rsucc + (1.0 - _lsucc) * (1.0 - _rsucc);
        default: return std::numeric_limits<double>::quiet_NaN();
        }
    }

    double cost() const noexcept override {
        switch (_cond) {
        case CONDS::AND: case CONDS::NAND:
            return _r2l ?
                _rcost + _lcost * _rsucc :
                _lcost + _rcost * _lsucc;
        case CONDS::OR: case CONDS::NOR:
            return _r2l ?
                _rcost + _lcost * (1 - _rsucc) :
                _lcost + _rcost * (1 - _lsucc);
        case CONDS::XOR: case CONDS::XNOR:
            return _lcost + _rcost + 2e-8;
        default: return std::numeric_limits<double>::quiet_NaN();
        }        
    }

    bool apply_blocked(iter_t& ent) override {
        if (!_left || !_right)
            return false;
        node<iter_t, sv_t> *fir, *sec;
        if (_r2l) {
            sec = _left.get(), fir = _right.get();
        } else {
            fir = _left.get(), sec = _right.get();
        }
        
        bool fir_res = fir->apply_blocked(ent);
        switch (_cond) {
        case CONDS::NOR:
            return !(fir_res || sec->apply_blocked(ent));
        case CONDS::OR:
            return fir_res || sec->apply_blocked(ent);
        case CONDS::NAND:
            return !(fir_res && sec->apply_blocked(ent));
        case CONDS::AND:
            return fir_res && sec->apply_blocked(ent);
        case CONDS::XNOR:
            return !(fir_res ^ sec->apply_blocked(ent));
        case CONDS::XOR:
            return fir_res ^ sec->apply_blocked(ent);
        }
        // Unreachable
        std::terminate();
    }
    
    tribool_bad apply(iter_t& ent) override {
        if (!_left || !_right)
            return tribool_bad::False;
        node<iter_t, sv_t> *fir, *sec;
        if (_r2l) {
            sec = _left.get(), fir = _right.get();
        } else {
            fir = _left.get(), sec = _right.get();
        }

        tribool_bad fir_res = fir->apply(ent);
        switch (_cond) {
        case CONDS::NOR:
            return fir_res == tribool_bad::True ?
                tribool_bad::False : !(fir_res | sec->apply(ent));
        case CONDS::OR:
            return fir_res == tribool_bad::True ?
                tribool_bad::True : fir_res | sec->apply(ent);
        case CONDS::NAND:
            return fir_res == tribool_bad::False ?
                tribool_bad::True : !(fir_res & sec->apply(ent));
        case CONDS::AND:
            return fir_res == tribool_bad::False ?
                tribool_bad::False : fir_res & sec->apply(ent);
        case CONDS::XNOR:
            return !(fir_res ^ sec->apply(ent));
        case CONDS::XOR:
            return fir_res ^ sec->apply(ent);
        }
        // Unreachable
        std::terminate();
    } 
    
    node<iter_t, sv_t>* clone() const override {
        return new cond_node<iter_t, sv_t>(*this);
    }
    node<iter_t, sv_t>* clone_deep() const override {
        cond_node<iter_t, sv_t>* res = new cond_node<iter_t, sv_t>(*this);
        res->setprev(_left->clone_deep(), true);
        res->setprev(_right->clone_deep(), false);
        return res;
    }
    void setprev(node<iter_t, sv_t>* heap_node, bool is_left) override {
        if (is_left)
            _left.reset(heap_node);
        else _right.reset(heap_node);
    }
 
    cond_node(CONDS cond) : _cond(cond) {}
};

// For testing purposes only: takes 1 param and ignores it.
// Always true
template <class iter_t, class sv_t>
struct __nextparam_tester : public truefalse_node<iter_t, sv_t> {
	bool ate = false;
	__nextparam_tester() : truefalse_node<iter_t, sv_t>(true) {}

	bool next_param(sv_t) override {
		if (ate) 
			return false;
		ate = true;
		return true;
	}
};

}
}

// Convience functions in constructing large predicate trees
template<class iter_t, class sv_t>
orie::pred_tree::cond_node<iter_t, sv_t>
operator&(const orie::pred_tree::node<iter_t, sv_t> &lhs,
          const orie::pred_tree::node<iter_t, sv_t> &rhs)
{
    orie::pred_tree::cond_node<iter_t, sv_t> res(orie::pred_tree::CONDS::AND);
    res.setprev(lhs.clone(), true);
    res.setprev(rhs.clone(), false);
    return res;
}

template<class iter_t, class sv_t>
orie::pred_tree::cond_node<iter_t, sv_t>
operator|(const orie::pred_tree::node<iter_t, sv_t> &lhs,
          const orie::pred_tree::node<iter_t, sv_t> &rhs)
{
    orie::pred_tree::cond_node<iter_t, sv_t> res(orie::pred_tree::CONDS::OR);
    res.setprev(lhs.clone(), true);
    res.setprev(rhs.clone(), false);
    return res;
}

template<class iter_t, class sv_t>
orie::pred_tree::cond_node<iter_t, sv_t>
operator^(const orie::pred_tree::node<iter_t, sv_t> &lhs,
          const orie::pred_tree::node<iter_t, sv_t> &rhs)
{
    orie::pred_tree::cond_node<iter_t, sv_t> res(orie::pred_tree::CONDS::XOR);
    res.setprev(lhs.clone(), true);
    res.setprev(rhs.clone(), false);
    return res;
}

template<class iter_t, class sv_t> 
orie::pred_tree::not_node<iter_t, sv_t>
operator~(const orie::pred_tree::node<iter_t, sv_t> &stk_node) {
    orie::pred_tree::not_node<iter_t, sv_t> res;
    res.setprev(stk_node.clone(), false);
    return res;
}

