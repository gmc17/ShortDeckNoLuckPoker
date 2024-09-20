# ♠️♦️ ShortDeckNoLuckPoker ♣️♥️

![Release](https://img.shields.io/badge/release-v0.0.1-blue)
[![license](https://img.shields.io/github/license/gmc17/ShortDeckNoLuckPoker?style=flat-square)](https://github.com/gmc17/ShortDeckNoLuckPoker/blob/master/LICENSE)

This project provides the first free, open-source tool for strategy analysis in Short Deck Poker.

## Introduction

Short Deck Poker, also known as Six Plus Hold'em, is a variant of Texas Hold'em played with a 36-card deck. The hand rankings are unchanged, except that a flush ranks higher than a full house in this version. This project utilizes the CFR+ algorithm, a variation of Counterfactual Regret Minimization, to efficiently compute near-optimal strategies for this game. Our implementation currently offers a text-based user interface for generating ranges in any spot.

## Installation and Use

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/ShortDeckNoLuckPoker.git
   cd ShortDeckNoLuckPoker
   ```

2. Compile the project:
   ```bash
   make
   ```

3. Run the program:
   ```bash
   make run
   ```

4. Follow the prompts in the command-line interface to analyze any spot you'd like!

## Requirements

- C++ compiler with C++20 support
- Make

## Features

- Complete with parameter input, tree builder, and tree explorer.

![Demo GIF](images/demo_final.gif)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the AGPL-3.0 License - see the [LICENSE.md](LICENSE.md) file for details.

## Authors

[Glen Cahilly](https://github.com/gmc17)

## Acknowledgements

We would like to thank the authors of the following papers, whose work has been instrumental in the development of this project:

1. Oskari Tammelin: [Solving Large Imperfect Information Games Using CFR+](https://arxiv.org/pdf/1407.5042). (2014)
   - This paper provided the foundation for the implementation of the CFR+ algorithm.

2. Johanson, M., Waugh, K., Bowling, M., Zinkevich, M.: [Accelerating Best Response Calculation in Large Extensive Games](https://cdn.aaai.org/ocs/ws/ws1014/7083-30526-1-PB.pdf). In: Proceedings of the Twenty-Second International Joint Conference on Artificial Intelligence (IJCAI), pp. 258-265 (2011)
   - We utilized the O(n^2) --> O(nlogn) speedup for terminal node evaluation as described in this paper.

