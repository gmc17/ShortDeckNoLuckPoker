#include "ars_table.h"
#include "game_state.h"
#include "constants.h"

ARSTable::ARSTable(): data(NUM_ROUNDS * NUM_RANKS * NUM_POCKET_PAIRS, 0.0f) {}

float& ARSTable::operator()(int round, int rank, int pocket_pair_id) {
    return data[round * NUM_RANKS * NUM_POCKET_PAIRS + rank * NUM_POCKET_PAIRS + pocket_pair_id];
}

const float& ARSTable::operator()(int round, int rank, int pocket_pair_id) const {
    return data[round * NUM_RANKS * NUM_POCKET_PAIRS + rank * NUM_POCKET_PAIRS + pocket_pair_id];
}

void ARSTable::save_to_file(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file for writing: " + filename);
    }

    // Write the entire data vector to the file
    file.write(reinterpret_cast<const char*>(data.data()), 
               data.size() * sizeof(float));

    if (!file) {
        throw std::runtime_error("Error writing to file: " + filename);
    }

    file.close();
}

void ARSTable::load_from_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file for reading: " + filename);
    }

    // Read the entire data vector from the file
    file.read(reinterpret_cast<char*>(data.data()),
              data.size() * sizeof(float));

    if (file.gcount() != data.size() * sizeof(float)) {
        throw std::runtime_error("Error reading from file: " + filename);
    }

    file.close();
}

std::mutex mtx;

void ars_worker_exhaustive_river(int start, int thread_id, ARSTable& ars_table, ARSTable& cum_sum_table, ARSTable& count_table) {
    uint32_t i=0;
    for (int p1=start; p1<start+1; p1++) {
        for (int p2=p1+1; p2<18; p2++) {
            for (int f1=0; f1<36; f1++) {
                for (int f2=f1+1; f2<36; f2++) {
                    for (int f3=f2+1; f3<36; f3++) {
                        for (int t=f3+1; t<36; t++) {
                            for (int r=t+1; r<36; r++) {
                                if ((p1!=f1) &&
                                    (p1!=f2) &&
                                    (p1!=f3) &&
                                    (p1!=t) &&
                                    (p1!=r) &&
                                    (p2!=f1) &&
                                    (p2!=f2) &&
                                    (p2!=f3) &&
                                    (p2!=t) &&
                                    (p2!=r)) {

                                    int p_id = pocket_id(p1, p2);
                                    std::array<uint32_t, 4> suit_cards = {0, 0, 0, 0};
                                    suit_cards[p1/9] |= (0b1 << (p1%9));
                                    suit_cards[p2/9] |= (0b1 << (p2%9));
                                    suit_cards[f1/9] |= (0b1 << ((f1%9) + 18));
                                    suit_cards[f2/9] |= (0b1 << ((f2%9) + 18));
                                    suit_cards[f3/9] |= (0b1 << ((f3%9) + 18));
                                    uint8_t turn = (((t/9) & 0x3) << 4) | ((t%9) & 0xF);
                                    uint8_t rivr = (((r/9) & 0x3) << 4) | ((r%9) & 0xF);
                                    GameState gs(suit_cards[0], suit_cards[1], suit_cards[2], suit_cards[3], turn, rivr, 1, 1, 1, 1, 0);
                                    int rank = gs.best_hand(0) / 100;
                                    {
                                        std::lock_guard<std::mutex> lock(mtx);
                                        count_table(2, rank, p_id) += 1;
                                        cum_sum_table(2, rank, p_id) += gs.rivr_hand_strength();
                                        ars_table(2, rank, p_id) = cum_sum_table(2, rank, p_id) / count_table(2, rank, p_id);
                                    }
                                    if ((i + 1) % 100000 == 0) std::cout << "Thread " << thread_id << " progress: " << i + 1 << "\n";
                                    if ((i + 1) % 1000000 == 0) {
                                        ars_table.save_to_file("ars_table.dat");
                                    }
                                    i++;
                                }
                            }
                        }   
                    }
                }
            }
        }
    }
}

void ars_worker_exhaustive_turn(int start, int thread_id, ARSTable& ars_table, ARSTable& cum_sum_table, ARSTable& count_table) {
    uint32_t i=0;
    for (int p1=start; p1<start+1; p1++) {
        for (int p2=p1+1; p2<18; p2++) {
            for (int f1=0; f1<36; f1++) {
                for (int f2=f1+1; f2<36; f2++) {
                    for (int f3=f2+1; f3<36; f3++) {
                        for (int t=f3+1; t<36; t++) {

                            if ((f1!=p1) && (f1!=p2) &&
                                (f2!=p1) && (f2!=p2) &&
                                (f3!=p1) && (f3!=p2) &&
                                (t!=p1) && (t!=p2)) {
                                
                                int p_id = pocket_id(p1, p2);
                                std::array<uint32_t, 4> suit_cards = {0, 0, 0, 0};
                                suit_cards[p1/9] |= (0b1 << (p1%9));
                                suit_cards[p2/9] |= (0b1 << (p2%9));
                                suit_cards[f1/9] |= (0b1 << ((f1%9) + 18));
                                suit_cards[f2/9] |= (0b1 << ((f2%9) + 18));
                                suit_cards[f3/9] |= (0b1 << ((f3%9) + 18));
                                uint8_t turn = (((t/9) & 0x3) << 4) | ((t%9) & 0xF);
                                GameState gs(suit_cards[0], suit_cards[1], suit_cards[2], suit_cards[3], turn, 0, 1, 1, 0, 1, 0);
                                int turn_rank = gs.best_hand(0) / 100;

                                for (int r=0; r<36; r++) {
                                    if ((r!=p1) && (r!=p2) && (r!=f1) && (r!=f2) && (r!=f3) && (r!=t)) {
                                        gs.rivr = (((r/9) & 0x3) << 4) | ((r%9) & 0xF);
                                        gs.rivr_history = 1;
                                        int rivr_rank = gs.best_hand(0) / 100;
                                        
                                        {
                                            std::lock_guard<std::mutex> lock(mtx);
                                            count_table(1, turn_rank, p_id) += 1;
                                            cum_sum_table(1, turn_rank, p_id) += ars_table(2, rivr_rank, p_id);
                                            ars_table(1, turn_rank, p_id) = cum_sum_table(1, turn_rank, p_id) / count_table(1, turn_rank, p_id);
                                        }
                                        if ((i + 1) % 1000000 == 0) std::cout << "Thread " << thread_id << " progress: " << i + 1 << "\n";
                                        if ((i + 1) % 10000000 == 0) {
                                            ars_table.save_to_file("ars_table.dat");
                                        }
                                        i++;
                                    }
                                }
                            }
                        }   
                    }
                }
            }
        }
    }
}

void ars_worker_exhaustive_flop(int start, int thread_id, ARSTable& ars_table, ARSTable& cum_sum_table, ARSTable& count_table) {
    uint32_t i=0;
    for (int p1=start; p1<start+1; p1++) {
        for (int p2=p1+1; p2<18; p2++) {
            for (int f1=0; f1<36; f1++) {
                for (int f2=f1+1; f2<36; f2++) {
                    for (int f3=f2+1; f3<36; f3++) {
                        if ((f1!=p1) && (f1!=p2) &&
                            (f2!=p1) && (f2!=p2) &&
                            (f3!=p1) && (f3!=p2)) {
                                
                            int p_id = pocket_id(p1, p2);
                            std::array<uint32_t, 4> suit_cards = {0, 0, 0, 0};
                            suit_cards[p1/9] |= (0b1 << (p1%9));
                            suit_cards[p2/9] |= (0b1 << (p2%9));
                            suit_cards[f1/9] |= (0b1 << ((f1%9) + 18));
                            suit_cards[f2/9] |= (0b1 << ((f2%9) + 18));
                            suit_cards[f3/9] |= (0b1 << ((f3%9) + 18));
                            GameState gs(suit_cards[0], suit_cards[1], suit_cards[2], suit_cards[3], 0, 0, 1, 0, 0, 1, 0);
                            int flop_rank = gs.best_hand(0) / 100;
                            
                            for (int t=0; t<36; t++) {
                                if ((t!=p1) && (t!=p2) && (t!=f1) && (t!=f2) && (t!=f3)) {
                                    gs.turn = (((t/9) & 0x3) << 4) | ((t%9) & 0xF);
                                    gs.turn_history = 1;
                                    int turn_rank = gs.best_hand(0) / 100;
                                    
                                    {
                                        std::lock_guard<std::mutex> lock(mtx);
                                        count_table(0, flop_rank, p_id) += 1;
                                        cum_sum_table(0, flop_rank, p_id) += ars_table(1, turn_rank, p_id);
                                        ars_table(0, flop_rank, p_id) = cum_sum_table(0, flop_rank, p_id) / count_table(0, flop_rank, p_id);
                                    }
                                    if ((i + 1) % 1000000 == 0) std::cout << "Thread " << thread_id << " progress: " << i + 1 << "\n";
                                    if ((i + 1) % 1000000 == 0) {
                                        ars_table.save_to_file("ars_table.dat");
                                    }
                                    i++;
                                }
                            }   
                        }
                    }
                }
            }
        }
    }
}

void generate_ARS_tables() {
    ARSTable cum_sum_table;
    ARSTable count_table; 
    ARSTable ars_table;   

    try {
        ars_table.load_from_file("ars_table.dat");
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    const int num_threads = 9;
    std::vector<std::thread> threads;

    for (int i = 0; (i < num_threads); ++i) {
        threads.emplace_back(ars_worker_exhaustive_river, i, i, std::ref(ars_table), std::ref(cum_sum_table), std::ref(count_table));
    }
    for (int i = 0; (i < num_threads); ++i) {
        threads.emplace_back(ars_worker_exhaustive_turn, i, i, std::ref(ars_table), std::ref(cum_sum_table), std::ref(count_table));
    }
    for (int i = 0; (i < num_threads); ++i) {
        threads.emplace_back(ars_worker_exhaustive_flop, i, i, std::ref(ars_table), std::ref(cum_sum_table), std::ref(count_table));
    }

    for (auto &thread : threads) {
        thread.join();
    }

    ars_table.save_to_file("ars_table.dat");
    std::cout << "Completed all iterations.\n";
}