#pragma once
#include <array>
#include <vector>
#include <variant>
#include <memory>
#include <cstdint>

#include "constants.h"
#include "game_state.h"
#include "info_set.h"

class Tree {
public:
	~Tree() = default;

    // Copy constructor
    Tree(const Tree&) = delete; 

    // Move constructor
    Tree(Tree&&) = default; 

    // Copy assignment operator
    Tree& operator=(const Tree&) = delete; 

    // Move assignment operator
    Tree& operator=(Tree&&) = default;

    class Node; // Forward declaration

    struct DecisionNode {
        std::array<std::array<std::vector<float>, NUM_CARDS>, NUM_CARDS> strategy_sum;
        std::array<std::array<std::vector<float>, NUM_CARDS>, NUM_CARDS> regret_sum;
        std::array<std::unique_ptr<Node>, MAX_ACTIONS> children;
        std::bitset<NUM_CARDS> card_marker;
        int actions;

        DecisionNode(int actions, const std::array<int, 5>& board_cards);
        std::array<float, MAX_ACTIONS> get_strategy(int c1, int c2) const;
        void update_strategy_sum(int c1, int c2, const std::array<float, MAX_ACTIONS>& strategy, float weight);
        void accumulate_regret(int action,
        					   const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& info_set_action_utilities,
        					   const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& info_set_utilities);
        inline bool has_card(int card) {
        	return card_marker[card - 1];
        }
    };

    struct ChanceNode {
        std::array<std::unique_ptr<Node>, NUM_CARDS> children;
        std::bitset<NUM_CARDS> card_marker;
        int num;

        ChanceNode(int num, const std::array<int, 5>& board_cards);
    };

    struct TerminalNode {
    	bool is_fold;
    	bool folding_player;
    	float pot;

    	TerminalNode(bool is_f, bool folding_p, float pot_size);
    };

    class Node : public std::variant<DecisionNode, ChanceNode, TerminalNode> {
    public:
        using std::variant<DecisionNode, ChanceNode, TerminalNode>::variant;

        bool is_decision_node() const {
            return std::holds_alternative<DecisionNode>(*this);
        }

        bool is_chance_node() const {
            return std::holds_alternative<ChanceNode>(*this);
        }

        bool is_terminal_node() const {
            return std::holds_alternative<TerminalNode>(*this);
        }

        DecisionNode& as_decision_node() {
            return std::get<DecisionNode>(*this);
        }

        ChanceNode& as_chance_node() {
            return std::get<ChanceNode>(*this);
        }

        TerminalNode& as_terminal_node() {
            return std::get<TerminalNode>(*this);
        }

        const DecisionNode& as_decision_node() const {
            return std::get<DecisionNode>(*this);
        }

        const ChanceNode& as_chance_node() const {
            return std::get<ChanceNode>(*this);
        }

        const TerminalNode& as_terminal_node() const {
            return std::get<TerminalNode>(*this);
        }
    };

    Tree(const GameState& root_state);
    Node* get_root() const { return root.get(); }
    Node* get_node(const std::vector<int>& history) const;

private:
    std::unique_ptr<Node> root;
    void build_tree(Node* node, const GameState& state);
};

int count_nodes(GameState state);