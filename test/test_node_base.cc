#include <gtest/gtest.h>
#include <orient/pred_tree/node.hpp>

TEST(baseNode, truefalse) {
    using namespace orie::pred_tree;
    auto node_true = truefalse_node<const int, std::string_view>(true);
    auto node_false = truefalse_node<const int, std::string_view>(false);

    EXPECT_TRUE(node_true.apply_blocked(0));
    EXPECT_FALSE(node_false.apply_blocked(0));

    EXPECT_FALSE(node_true.next_param(""));
    EXPECT_FALSE(node_false.next_param(""));
}

TEST(baseNode, condsApply) {
    using namespace orie::pred_tree;
    auto node_true = truefalse_node<const int, std::string_view>(true);
    auto node_false = truefalse_node<const int, std::string_view>(false);

    auto node_cond = node_true & node_false;
    EXPECT_FALSE(node_cond.apply_blocked(0));
    node_cond = node_true | node_false;
    EXPECT_TRUE(node_cond.apply_blocked(0));

    node_cond = node_false & node_true;
    EXPECT_FALSE(node_cond.apply_blocked(0));
    node_cond = node_false | node_true;
    EXPECT_TRUE(node_cond.apply_blocked(0));

    node_cond = node_true & node_true;
    EXPECT_TRUE(node_cond.apply_blocked(0));
    node_cond = node_false & node_false;
    EXPECT_FALSE(node_cond.apply_blocked(0));

    node_cond = node_true | node_true;
    EXPECT_TRUE(node_cond.apply_blocked(0));
    node_cond = node_false | node_false;
    EXPECT_FALSE(node_cond.apply_blocked(0));
}

TEST(baseNode, condsObserver) {
    using namespace orie::pred_tree;
    auto node_true = truefalse_node<const int, std::string_view>(true);
    auto node_false = truefalse_node<const int, std::string_view>(false);
    EXPECT_TRUE(node_true.communicative());
    EXPECT_DOUBLE_EQ(1.0, node_true.success_rate());
    EXPECT_TRUE(node_false.communicative());
    EXPECT_DOUBLE_EQ(0.0, node_false.success_rate());

    auto node_cond = node_true & node_false;
    EXPECT_FALSE(node_cond.next_param(""));
    node_cond.update_cost();
    EXPECT_TRUE(node_cond.communicative());
    EXPECT_DOUBLE_EQ(0.0, node_cond.success_rate());

    node_cond = node_false | node_true;
    EXPECT_FALSE(node_cond.next_param(""));
    node_cond.update_cost();
    EXPECT_TRUE(node_cond.communicative());
    EXPECT_DOUBLE_EQ(1.0, node_cond.success_rate());

    EXPECT_DOUBLE_EQ(node_true.cost(), node_false.cost());
    EXPECT_LT(node_true.cost(), node_cond.cost());
}

TEST(baseNode, not) {
    using namespace orie::pred_tree;
    auto node_true = truefalse_node<const int, std::string_view>(true);
    auto node_false = truefalse_node<const int, std::string_view>(false);

    auto node_not = ~node_true;
    node_not.update_cost();
    EXPECT_FALSE(node_not.next_param(""));
    EXPECT_FALSE(node_not.apply_blocked(0));
    EXPECT_TRUE(node_not.communicative());
    EXPECT_DOUBLE_EQ(0.0, node_not.success_rate());

    node_not = ~(node_true & node_false);
    EXPECT_TRUE(node_not.apply_blocked(0));
    node_not = ~(node_true | node_false);
    EXPECT_FALSE(node_not.apply_blocked(0));

    node_not = ~~(node_true & node_false);
    EXPECT_FALSE(node_not.apply_blocked(0));
    node_not = ~~(node_true | node_false);
    EXPECT_TRUE(node_not.apply_blocked(0));
}

TEST(baseNode, nextParam) {
    using namespace orie::pred_tree;
    auto node_param = __nextparam_tester<const int, std::string_view>();
    EXPECT_TRUE(node_param.next_param(""));
    EXPECT_FALSE(node_param.next_param(""));
    EXPECT_TRUE(node_param.apply_blocked(0));
}

TEST(baseNode, updateCost) {
    using namespace orie::pred_tree;
    auto node_true = truefalse_node<const int, std::string_view>(true);
    auto node_false = truefalse_node<const int, std::string_view>(false);

    auto node_cond = node_true | node_false;
    for (size_t i = 0; i < 15; ++i)
        node_cond = node_cond & node_cond;

    node_cond.update_cost();
    EXPECT_TRUE(node_cond.apply_blocked(0));
    EXPECT_DOUBLE_EQ(1.0, node_cond.success_rate());
    double time_slow = node_cond.cost();

    (node_cond = node_cond & node_false).update_cost();
    EXPECT_FALSE(node_cond.apply_blocked(0));
    EXPECT_DOUBLE_EQ(0.0, node_cond.success_rate());
    double time_fast = node_cond.cost();
    EXPECT_LT(time_fast, time_slow);
}
