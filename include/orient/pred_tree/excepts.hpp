#pragma once
#include <stdexcept>
#include <orient/util/charconv_t.hpp>

namespace orie {
namespace pred_tree {
// PRED-TREE Error Handling

struct invalid_pred_tree : public std::logic_error {
    invalid_pred_tree(const char* w) 
        : std::logic_error(w) {}
};

struct empty_parentheses : public invalid_pred_tree {
    empty_parentheses()
        : invalid_pred_tree("Empty Parentheses are not allowed.") {}
};

struct parentheses_mismatch : public invalid_pred_tree {
    parentheses_mismatch(bool is_left)
        : invalid_pred_tree(is_left ? "Too much '('" : "Too mush ')'") {}
};

class unknown_node_name : public invalid_pred_tree {
    char msg[64] = "Unknown Predicate: ";

public:
    template <typename sv_t>
    unknown_node_name(sv_t pred_name) : invalid_pred_tree("") {
        orie::strncpy(msg + 19, pred_name.data(), 
            std::min(pred_name.size(), size_t(40)));
    }

    const char* what() const noexcept override { return msg; }
};

class missing_bridge : public invalid_pred_tree {
    char msg[64] = "Expected a bridge before ";

public:
    template <typename sv_t>
    missing_bridge(sv_t pred_name) : invalid_pred_tree("") {
        orie::strncpy(msg + 25, pred_name.data(), 
            std::min(pred_name.size(), size_t(30)));
    }

    const char* what() const noexcept override { return msg; }
};


// Thrown when a bridge node is not connected to 2 sub nodes
// or a modifier has no sub node.
// Ex: ... -o EOF, ... -not EOF
class missing_predicate : public invalid_pred_tree {
    char msg[64] = "Expected a predicate";

public:
    template <typename sv_t>
    missing_predicate(sv_t brid_name, bool before) : invalid_pred_tree("") {
        before ? ::strcat(msg, " before ") : ::strcat(msg, " after ");
        orie::strncat(msg, brid_name.data(), std::min(brid_name.size(), size_t(30)));
    }

    const char* what() const noexcept override { return msg; }
};

// Node Argument Errors
struct invalid_node_arg : public std::logic_error {
    invalid_node_arg(const char* w) 
        : std::logic_error(w) {}
};

// e.g. -size aaaaa
class not_a_number : public invalid_node_arg {
    char msg[64] = "Not a valid number: ";

public:
    template <typename sv_t>
    not_a_number(sv_t param) : invalid_node_arg("") {
        orie::strncat(msg, param.data(), param.size() > 40 ? 40 : param.size());
    }

    const char* what() const noexcept override { return msg; }
};

// Thrown when an invalid argument is passed to a node
// e.g. -name --strsrr haystack
class invalid_param_name : public invalid_node_arg {
    char msg[64] = "";

public:
    template <typename sv_t>
    invalid_param_name(sv_t param, sv_t node) : invalid_node_arg("") {
        orie::strncpy(msg, param.data(), std::min(size_t(20), param.size()));
        orie::strcat(msg, " is invalid for ");
        orie::strncat(msg, node.data(), std::min(size_t(25), node.size()));
        msg[63] = '\0';
    }

    const char* what() const noexcept override { return msg; }
};

class already_initialized_node : public invalid_node_arg {
    char msg[64] = "Node is already initialized: ";

public:
    already_initialized_node(std::string_view param) : invalid_node_arg("") {
        orie::strncat(msg, param.data(), param.size() > 40 ? 40 : param.size());
    }

    const char* what() const noexcept override { return msg; }
};

class uninitialized_node : public invalid_node_arg {
    char msg[64] = "Node is not initialized: ";

public:
    template <typename sv_t>
    uninitialized_node(sv_t param) : invalid_node_arg("") {
#ifdef __GNUC_STDC_INLINE__
#pragma GCC diagnostic push
// GCC warns that `msg` is larger than `param.size()`, which is intended.
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
        orie::strncat(msg, param.data(), param.size() > 30 ? 30 : param.size());
#ifdef __GNUC_STDC_INLINE__
#pragma GCC diagnostic pop
#endif
    }

    const char* what() const noexcept override { return msg; }
};

// Thrown by -quitmod; caught by async_job
struct quitted : public std::runtime_error {
    quitted(const char* what) : std::runtime_error(what) {}
};

}
}
