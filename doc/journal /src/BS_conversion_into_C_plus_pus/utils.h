#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <climits>
#include <limits>
#include <iostream>
#include <stdexcept>

// ====================== Parameters and Heuristic Types ======================

enum class HeuristicType {
    H1,
    H2,
    H5,
    H8
};

struct Parameters {
    int beam_width = 100;
    HeuristicType heuristic = HeuristicType::H1;
    int max_iters = 10000;
    int time_limit_sec = 60;

    // optional: two-phase BS
    HeuristicType heuristic_prime = HeuristicType::H1;
    int beam_width_prime = 10;
};

extern Parameters params;

// ====================== Heuristic helpers ======================

HeuristicType parseHeuristic(const std::string& h);
std::string heuristicToString(HeuristicType h);

// ====================== Feasibility ======================

bool check_feasibility_n(
    const std::vector<std::vector<int>>& trace,
    const std::vector<std::vector<int>>& gaps,
    bool verbose = false
);

// ====================== Lower / Upper bounds ======================

int remaining_lb(
    const std::vector<int>& pos,
    const std::vector<std::string>& sequences
);

int h5_upper_bound(
    const std::vector<int>& pos,
    const std::vector<std::unordered_map<char, std::vector<int>>>& C_h5,
    const std::vector<char>& Sigma_h5,
    const std::vector<std::string>& sequences
);

// ====================== BACKWARD h5 ======================

int h5_backward(
    const std::vector<int>& pos,
    const std::vector<std::unordered_map<char, std::vector<int>>>& C_h5,
    const std::vector<char>& Sigma_h5
);

// ====================== Probability heuristic ======================

double probability_based_heuristic(
    const std::vector<int>& pos,
    const std::vector<std::vector<double>>& P,
    const std::vector<std::string>& sequences,
    int k
);

// ====================== Helpers ======================

std::unordered_map<char, int> first_valid_indices(
    const std::string& seq,
    int start_idx,
    const std::vector<int>& gap_arr,
    int prev_idx
);

#endif // UTILS_H
