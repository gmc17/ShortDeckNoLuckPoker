#include "cfr.h"
#include <unordered_map>
#include <algorithm>

std::unordered_map<GameState, std::array<float, 3>> regret_sum;
std::unordered_map<GameState, std::array<float, 3>> strategy_sum;

// Define the output operator for std::array<float, 3>
std::ostream& operator<<(std::ostream& os, const std::array<float, 3>& arr) {
    os << "[";
    for (size_t i = 0; i < arr.size(); ++i) {
        os << arr[i];
        if (i != arr.size() - 1) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

std::array<float, 3> get_strategy(const GameState& info_set) {
    if (!regret_sum.contains(info_set)) {
        regret_sum[info_set] = {0, 0, 0};
        strategy_sum[info_set] = {0, 0, 0};
    }

    std::array<float, 3> regrets = regret_sum[info_set];
    int actions = info_set.num_actions();
    int normalizing_sum = 0;

    // get positive regrets
    for (int i=0; i<actions; i++) {
        regrets[i] = std::max(0.0f, regrets[i]);
        normalizing_sum += regrets[i];
    }
    
    if (normalizing_sum > 0) {
        for (int i=0; i<actions; i++) regrets[i] /= normalizing_sum;
    } else {
        for (int i=0; i<actions; i++) regrets[i] = 1.0 / actions;
    }

    return regrets;
}


/*

// Function to traverse the game tree
function traverseTree(node, p0, p1):
    if node is a terminal node:
        return utility for the current player
    
    if node is a chance node:
        // Handle chance nodes
        for each possible outcome o:
            traverseTree(node.next[o], p0 * P(o), p1 * P(o))
        return
    
    I = information set corresponding to node
    strategy = getStrategy(I, regretSum[I])

    if node is a player 0 node:
        util = 0
        nodeUtil = 0
        for each action a in A:
            nextNode = node.next[a]
            childUtil = traverseTree(nextNode, p0 * strategy[a], p1)
            util[a] = childUtil
            nodeUtil += strategy[a] * childUtil

        // Update regrets
        for each action a in A:
            regretSum[I][a] += (util[a] - nodeUtil) * p1
            strategySum[I][a] += p0 * strategy[a]
        
        return nodeUtil
    
    else if node is a player 1 node:
        util = 0
        nodeUtil = 0
        for each action a in A:
            nextNode = node.next[a]
            childUtil = traverseTree(nextNode, p0, p1 * strategy[a])
            util[a] = childUtil
            nodeUtil += strategy[a] * childUtil

        // Update regrets
        for each action a in A:
            regretSum[I][a] += (util[a] - nodeUtil) * p0
            strategySum[I][a] += p1 * strategy[a]
        
        return nodeUtil

*/