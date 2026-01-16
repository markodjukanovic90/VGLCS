#include "utils.h"

// ====================== Global params ======================
Parameters params;

// ====================== Heuristic parsing ======================

HeuristicType parseHeuristic(const std::string& h) {
    if (h == "h1") return HeuristicType::H1;
    if (h == "h2") return HeuristicType::H2;
    if (h == "h5") return HeuristicType::H5;
    if (h == "h8") return HeuristicType::H8;
    throw std::invalid_argument("Unknown heuristic: " + h);
}

std::string heuristicToString(HeuristicType h) {
    switch (h) {
        case HeuristicType::H1: return "h1";
        case HeuristicType::H2: return "h2";
        case HeuristicType::H5: return "h5";
        case HeuristicType::H8: return "h8";
    }
    return "unknown";
}

// ====================== Feasibility ======================

 bool check_feasibility(
    const std::vector<std::vector<int>>& trace,
    const std::vector<std::vector<int>>& gaps,
    bool verbose
) {
    if (trace.size() <= 1) return true;

    int N = static_cast<int>(gaps.size());

    for (size_t step = 1; step < trace.size(); ++step) {
        const auto& prev = trace[step - 1];
        const auto& cur  = trace[step];

        for (int k = 0; k < N; ++k) {
            int diff  = cur[k] - prev[k];
            int limit = gaps[k][cur[k]] + 1;
            //std::cout <<  cur[k] << " " << prev[k] << "\n";
            if (diff > limit) {
                if (verbose) {
                    std::cerr << "Jump too large in seq#" << (k + 1)
                              << ": " << diff << " > " << limit << "\n";
                }
                return false;
            }
        }
    }
    return true;
}

// ====================== Remaining LB ======================

int remaining_lb(
    const std::vector<int>& pos,
    const std::vector<std::string>& sequences
) {
    int min_rem = INT_MAX;
    for (size_t i = 0; i < sequences.size(); ++i) {
        int rem = static_cast<int>(sequences[i].size()) - pos[i] - 1;
        min_rem = std::min(min_rem, rem);
    }
    return min_rem;
}

// ====================== H5 upper bound ======================

int h5_upper_bound(
    const std::vector<int>& pos,
    const std::vector<std::unordered_map<char, std::vector<int>>>& C_h5,
    const std::vector<char>& Sigma_h5,
    const std::vector<std::string>& sequences
) {
    int m = static_cast<int>(C_h5.size());
    std::vector<int> j_starts(m);

    for (int i = 0; i < m; ++i)
        j_starts[i] = std::min(pos[i] + 1,
                               static_cast<int>(sequences[i].size()));

    int ub = 0;
    for (char a : Sigma_h5) {
        int mn = INT_MAX;
        for (int i = 0; i < m; ++i) {
            const auto& vec = C_h5[i].at(a);
            mn = std::min(mn, vec[j_starts[i]]);
        }
        ub += mn;
    }
    return ub;
}

// ====================== H5 BACKWARD (FINAL, CORRECT) ======================

int h5_backward(
    const std::vector<int>& pos,
    const std::vector<std::unordered_map<char, std::vector<int>>>& C_h5,
    const std::vector<char>& Sigma_h5
) {
    const int m = static_cast<int>(pos.size());
    int h = 0;

    for (char a : Sigma_h5) {
        int min_count = std::numeric_limits<int>::max();

        for (int i = 0; i < m; ++i) {
            int p_i = pos[i];

            if (p_i < 0) {
                min_count = 0;
                break;
            }

            const std::vector<int>& suffix_vec = C_h5[i].at(a);

            int total_count  = suffix_vec[0];
            int suffix_count = suffix_vec[p_i + 1];

            int prefix_count = total_count - suffix_count;
            min_count = std::min(min_count, prefix_count);
        }
        h += min_count;
    }
    return h;
}

// ====================== Probability heuristic ======================

double probability_based_heuristic(
    const std::vector<int>& pos,
    const std::vector<std::vector<double>>& P,
    const std::vector<std::string>& sequences,
    int k
) {
    double score = 1.0;
    int m = static_cast<int>(pos.size());

    for (int i = 0; i < m; ++i) {
        int p_i = pos[i];
        if (p_i < 0 || p_i >= static_cast<int>(P[i].size()))
            return 0.0;

        score *= P[i][sequences[i].size() - p_i + 1];
    }
    return score;
}

// ====================== first_valid_indices ======================

std::unordered_map<char, int> first_valid_indices(
    const std::string& seq,
    int start_idx,
    const std::vector<int>& gap_arr,
    int prev_idx
) {
    std::unordered_map<char, int> mapping;

    for (size_t j = start_idx; j < seq.size(); ++j) {
        if (prev_idx != -1 &&
            static_cast<int>(j - prev_idx) > gap_arr[j] + 1)
            continue;

        char c = seq[j];
        if (!mapping.count(c))
            mapping[c] = static_cast<int>(j);
    }
    return mapping;
}
