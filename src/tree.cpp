#include "tree.h"

Tree::DecisionNode::DecisionNode(
    bool p, 
    int num_actions, 
    const std::array<int, 5>& board_cards) {
    
    player = p;
    actions = num_actions;

    for (auto& row : strategy_sum) {
        for (auto& cell : row) {
            cell.resize(actions, 0.0f);
        }
    }
    for (auto& row : regret_sum) {
        for (auto& cell : row) {
            cell.resize(actions, 0.0f);
        }
    }

    for (int i=0; i<NUM_CARDS; i++) card_marker[i] = false;
    for (const auto& card : board_cards) {
        if (card >= 1 && card <= NUM_CARDS) card_marker[card - 1] = true;
    }
}

Tree::ChanceNode::ChanceNode(
    int num_in_deck, 
    const std::array<int, 5>& board_cards) {
    
    num = num_in_deck;

    for (int i=0; i<NUM_CARDS; i++) card_marker[i] = false;
    for (const auto& card : board_cards) {
        if (card >= 1 && card <= NUM_CARDS) card_marker[card - 1] = true;
    }
}

Tree::TerminalNode::TerminalNode(bool is_f, bool folding_p, float pot_size, const std::array<int, 5>& board_cards) {
    is_fold = is_f;
    folding_player = folding_p;
    pot = pot_size;
    trn = board_cards[3];
    rvr = board_cards[4];
    for (int i=0; i<NUM_CARDS; i++) card_marker[i] = false;
    for (const auto& card : board_cards) {
        if (card >= 1 && card <= NUM_CARDS) card_marker[card - 1] = true;
    }
}

std::array<float, MAX_ACTIONS> Tree::DecisionNode::get_strategy(int c1, int c2) const {
    std::array<float, MAX_ACTIONS> strategy;
    const auto& regrets = regret_sum[c1 - 1][c2 - 1];
    float normalizing_sum = 0.0f;
    for (int a = 0; a < actions; ++a) {
        strategy[a] = std::max(regrets[a], 0.0f);
        normalizing_sum += strategy[a];
    }
    if (normalizing_sum > 1e-7) {
        for (int a = 0; a < actions; ++a) {
            strategy[a] /= normalizing_sum;
        }
    } else {
        float uniform = 1.0f / actions;
        for (int a = 0; a < actions; ++a) {
            strategy[a] = uniform;
        }
    }
    return strategy;
}

std::array<float, MAX_ACTIONS> Tree::DecisionNode::get_average_strategy(int c1, int c2) const {
    std::array<float, MAX_ACTIONS> avg_strategy;
    const auto& strat_sum = strategy_sum[c1 - 1][c2 - 1];
    float normalizing_sum = 0.0f;
    for (int a = 0; a < actions; ++a) {
        normalizing_sum += strat_sum[a];
    }
    if (normalizing_sum > 1e-2) {
        for (int a = 0; a < actions; ++a) {
            avg_strategy[a] = strat_sum[a] / normalizing_sum;
        }
    } else {
        float uniform_prob = 1.0f / actions;
        std::fill(avg_strategy.begin(), avg_strategy.begin() + actions, uniform_prob);
    }
    return avg_strategy;
}

void Tree::DecisionNode::update_strategy_sum(int c1, int c2, const std::array<float, MAX_ACTIONS>& strategy, float weight) {
    for (int a = 0; a < actions; ++a) {
        strategy_sum[c1 - 1][c2 - 1][a] += weight * strategy[a];
    }
}

void Tree::DecisionNode::accumulate_regret(int action,
                                           const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& info_set_action_utilities,
                                           const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& info_set_utilities) {
    for (int c1=1; c1<=NUM_CARDS; c1++) {
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            regret_sum[c1 - 1][c2 - 1][action] = std::max(regret_sum[c1 - 1][c2 - 1][action] + info_set_action_utilities[c1 - 1][c2 - 1] - info_set_utilities[c1 - 1][c2 - 1], 0.0f);
        }
    }
}

Tree::Tree(const GameState& root_state) : root(nullptr) {
    std::array<int, 5> board_cards = {root_state.fp1, root_state.fp2, root_state.fp3, root_state.trn, root_state.rvr};
    if (root_state.is_terminal) {
        root = std::make_unique<Node>(TerminalNode(root_state.is_fold(), root_state.player, root_state.pot_size, board_cards));
    } else if (root_state.is_chance()) {
        int num = root_state.num_in_deck();
        root = std::make_unique<Node>(ChanceNode(num, board_cards));
    } else {
        bool player = root_state.player;
        int actions = root_state.num_actions();
        root = std::make_unique<Node>(DecisionNode(player, actions, board_cards));
    }
    build_tree(root.get(), root_state);
}

Tree::Node* Tree::get_node(const std::vector<int>& history) const {
    Node* current = root.get();
    for (int i = 0; i < history.size(); i++) {
        int action = history[i];

        if (std::holds_alternative<DecisionNode>(*current)) {
            auto& decision_node = std::get<DecisionNode>(*current);
            if (action >= 0 && action < MAX_ACTIONS && decision_node.children[action]) {
                current = decision_node.children[action].get();
            } else {
                return nullptr; // Invalid action
            }
        } else if (std::holds_alternative<ChanceNode>(*current)) {
            auto& chance_node = std::get<ChanceNode>(*current);
            if (action >= 1 && action <= NUM_CARDS && chance_node.children[action - 1]) {
                current = chance_node.children[action - 1].get();
            } else {
                return nullptr; // Invalid chance outcome
            }
        } else if (std::holds_alternative<TerminalNode>(*current)) {
            // If we've reached a terminal node and it's the last action in the history,
            // return the terminal node. Otherwise, return nullptr.
            return (i == history.size() - 1) ? current : nullptr;
        } else {
            return nullptr; // Unexpected node type
        }
    }
    return current;
}

void Tree::build_tree(Node* node, const GameState& state) {

    if (state.is_terminal) return;

    auto create_child_node = [](const GameState& new_state) -> std::unique_ptr<Node> {
        std::array<int, 5> board_cards = {new_state.fp1, new_state.fp2, new_state.fp3, new_state.trn, new_state.rvr};
        if (new_state.is_terminal) {
            return std::make_unique<Node>(TerminalNode(new_state.is_fold(), new_state.player, new_state.pot_size, board_cards));
        } else if (new_state.is_chance()) {
            return std::make_unique<Node>(ChanceNode(new_state.num_in_deck(), board_cards));
        } else {
            return std::make_unique<Node>(DecisionNode(new_state.player, new_state.num_actions(), board_cards));
        }
    };

    if (state.is_chance()) {
        auto& chance_node = std::get<ChanceNode>(*node);
        for (int c=1; c<=NUM_CARDS; c++) {
            if (!state.has_card(c)) {
                GameState new_state = state;
                new_state.deal_card(c);
                chance_node.children[c - 1] = create_child_node(new_state);
                build_tree(chance_node.children[c - 1].get(), new_state);
            }
        }
    } else {
        auto& decision_node = std::get<DecisionNode>(*node);
        for (int a=0; a<state.num_actions(); a++) {
            GameState new_state = state;
            new_state.apply_index(a);
            decision_node.children[a] = create_child_node(new_state);
            build_tree(decision_node.children[a].get(), new_state);
        }
    }
}

float estimate_tree_memory(GameState state) {
    // Terminal nodes in tree use about 20 bytes of memory
    if (state.is_terminal) return 20.0f;

    int res = 0.0f;

    // Chance
    if (state.is_chance()) {
        for (int c=1; c<=NUM_CARDS; c++) {
            if (!state.has_card(c)) {
                GameState new_state = state;
                new_state.deal_card(c);

                res += 20.0f + estimate_tree_memory(new_state);
            }
        }
        return res;
    }

    int actions = state.num_actions();

    // Action
    for (int a=0; a<actions; a++) {
        GameState new_state = state;
        new_state.apply_index(a);

        // 4 bytes in float
        res += 4 * NUM_CARDS * NUM_CARDS * MAX_ACTIONS + estimate_tree_memory(new_state);
    }

    return res;
}