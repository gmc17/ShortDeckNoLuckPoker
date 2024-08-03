# ShortDeckNoLuckPoker
![Release](https://img.shields.io/badge/release-v0.0.1-blue)
[![license](https://img.shields.io/github/license/gmc17/ShortDeckNoLuckPoker?style=flat-square)](https://github.com/gmc17/ShortDeckNoLuckPoker/blob/master/LICENSE)

This project implements the AS-MCCFR algorithm to compute Game Theory Optimal (GTO) strategies for heads-up Short Deck Poker. It aims to provide a free, open-source tool for strategy analysis in this increasingly popular poker variant.

## Introduction
Short Deck Poker, also known as Six Plus Hold'em, is a variant of Texas Hold'em played with a 36-card deck. The hand rankings are unchanged, except that in this version, a flush ranks higher than a full house. This project utilizes the AS-MCCFR algorithm, a variation of Counterfactual Regret Minimization, to compute near-optimal strategies for this game. Our implementation currently offers both a playable AI opponent and a tool for generating ranges. We are actively working on making the interface more user-friendly.

## Installation and Training the AI
To play against the AI, you need to first train it. Follow these steps:

1. Clone the repository: 
```bash
git clone https://github.com/yourusername/ShortDeckNoLuckPoker.git
cd ShortDeckNoLuckPoker
```

2. Compile the project: ```make```

3. Generate the ARS table (if ```ars_table.dat``` is not already present in your project directory):
```make generate-ars```
*Note:* this process can be time-consuming and only needs to be done once. It is highly recommended to simply use the file we uploaded to this page.

5. Train the AI:
```./shortdeck train [iterations]```
Replace `[iterations]` with the number of training iterations you want. For example:
```./shortdeck train 1000000```.
If you don't specify the number of iterations, it will use a default value of 1,000,000.

## Playing Against the AI

1. Play against the computer:
After training, start a game with:
```./shortdeck play```

2. Follow the prompts in the command-line interface to play your game against the AI.

Note: You can further train the AI at any time by running the ```train``` command again. The AI will start from its current state and continue training for the specified number of iterations.

3. You will be prompted to input actions during gameplay. Enter an integer from 1-7, corresponding to the following actions:

| Input | Action                    |
|-------|---------------------------|
| 1     | Fold / Check              |
| 2     | Bet 0.50x pot / Raise 2.00x |
| 3     | Bet 0.75x pot / Raise 2.75x |
| 4     | Bet 1.00x pot / Raise 3.50x |
| 5     | Bet 2.00x pot / Raise 5.00x |
| 6     | All-in                    |
| 7     | Call                      |

## Range Generation
The program can generate output its ranges for any position/action in the game. For example, here is its SB preflop fold range:

![SB Preflop Fold Range](sb_preflop_fold_range.png)

To generate custom ranges, you can use the `GameState::print_range()` function. We are working on making this feature more user-friendly in future updates.

## Results
After training for 30 million iterations, we evaluated our strategy using the Local Best Response (LBR) algorithm. The results are as follows:

- Approximate lower bound for strategy exploitability: 270 ± 50 mBB/h (milli big-blinds per hand)
- Training time: 6hrs
- Hardware used: 2018 MacBook Pro, 2.2 GHz 6-Core Intel Core i7, 16 GB 2400 MHz DDR4 RAM

This exploitability measure indicates that our strategy is highly competitive, with expected losses limited to less than 30 big blinds per 100 hands against a theoretical perfect opponent. For context, professional poker players often aim for win rates of 5-10 big blinds per 100 hands against human opponents.

## Requirements
* C++ compiler with C++20 support
* Make

## Contributing
Contributions are welcome! Please feel free to submit a Pull Request.

## License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.

## Authors
[Glen Cahilly](https://github.com/gmc17)

## Acknowledgements

We would like to thank the authors of the following papers, whose work has been instrumental in the development of this project:

[1]. Burch, N., Lanctot, M., Szafron, D., Gibson, R.G.: [Efficient Monte Carlo counterfactual regret minimization in games with many player actions](https://proceedings.neurips.cc/paper_files/paper/2012/file/3df1d4b96d8976ff5986393e8767f5b2-Paper.pdf). In: Advances in Neural Information Processing Systems, pp. 1880–1888 (2012)
   - This paper provided the foundation for our implementation of the AS-MCCFR algorithm.

[2]. Johanson, M., Waugh, K., Bowling, M., Zinkevich, M.: [Accelerating Best Response Calculation in Large Extensive Games](https://cdn.aaai.org/ocs/ws/ws1014/7083-30526-1-PB.pdf). In: Proceedings of the Twenty-Second International Joint Conference on Artificial Intelligence (IJCAI), pp. 258-265 (2011)
   - We utilized the Average Rank Strength concept from this paper to speed up our game abstraction computation.

[3]. Teofilo, L.F., Reis, L.P., Cardoso, H.L.: [Speeding-Up Poker Game Abstraction Computation: Average Rank Strength](https://cdn.aaai.org/ocs/ws/ws1014/7083-30526-1-PB.pdf). In: AAAI Workshop on Computer Poker and Imperfect Information (2013)
   - This work provided additional insights into optimizing the Average Rank Strength calculations for poker game abstractions.
