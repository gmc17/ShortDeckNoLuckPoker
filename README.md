# ShortDeckNoLuckPoker

An implementation of Average Sampling Monte Carlo Counterfactual Regret Minimization (AS-MCCFR) for short deck texas hold-em.

## Introduction

This project utilizes the AS-MCCFR algorithm to compute game theory optimal (GTO) strategies for shortdeck Heads-up Poker.

## Playing the computer

Currently, only terminal usage is supported. Run 'make run' to play against the computer. You will be prompted to input an action. This should be an integer from 1-7, inclusive.

Actions:
1. Fold / Check
2. Bet 0.50x pot / Raise 2.00x
3. Bet 0.75x pot / Raise 2.75x
4. Bet 1.00x pot / Raise 3.50x
5. Bet 2.00x pot / Raise 5.00x
6. All-in
7. Call

## Generated ranges
![SB Preflop Fold Range](sb_preflop_fold_range.png)

The program can generate ranges for any position. Work is being done to make this more user-friendly, but currently you can use the GameState::print_range() function to do so.

## Authors

[Glen Cahilly](https://github.com/gmc17)

## Acknowledgments

[Efficient Monte Carlo Counterfactual Regret Minimization in Games with Many Player Actions](https://proceedings.neurips.cc/paper_files/paper/2012/file/3df1d4b96d8976ff5986393e8767f5b2-Paper.pdf)

[Speeding-Up Poker Game Abstraction Computation:
Average Rank Strength ](https://cdn.aaai.org/ocs/ws/ws1014/7083-30526-1-PB.pdf)
