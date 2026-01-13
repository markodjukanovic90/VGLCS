#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <climits>
#include <iostream>
#include "Node.h"

// ====================== Check feasibility ======================
inline bool check_feasibility_n(
    const std::vector<std::vector<int>>& trace,
    const std::vector<std::vector<int>>& gaps,
    bool verbose = false
) {
    if (trace.size() <= 1) return true;

    int N = static_cast<int>(gaps.size());
    for (size_t step = 1; step < trace.size(); ++step) {
        const auto& prev = trace[step - 1];
        const auto& cur  = trace[step];

        for (int k = 0; k < N; ++k) {
            int diff  = cur[k] - prev[k];
            int limit = gaps[k][cur[k]] + 1;
            if (diff > limit) {
                if (verbose) {
                    std::cerr << "Jump too large in seq#" << (k + 1)
                              << ": " << diff << " > " << limit
                              << " (from " << prev[k] << " -> " << cur[k] << ")\n";
                }
                return false;
            }
        }
    }
    return true;
}

// ====================== Remaining lower bound ======================
inline int remaining_lb(const Node& node, const std::vector<std::string>& sequences) {
    int min_rem = INT_MAX;
    for (size_t i = 0; i < sequences.size(); ++i) {
        int rem = static_cast<int>(sequences[i].size()) - node.pos[i] - 1;
        if (rem < min_rem) min_rem = rem;
    }
    return min_rem;
}

// ====================== H5 upper bound ======================
inline int h5_upper_bound(
    Node* node,
    const std::vector<std::unordered_map<char, std::vector<int>>>& C_h5,
    const std::vector<char>& Sigma_h5,
    const std::vector<int>& seq_lens
) {
    int m = static_cast<int>(C_h5.size());
    std::vector<int> j_starts(m);
    for (int i = 0; i < m; ++i) {
        j_starts[i] = std::min(node->pos[i] + 1, seq_lens[i]);
    }

    int ub = 0;
    for (char a : Sigma_h5) {
        int mn = INT_MAX;
        for (int i = 0; i < m; ++i) {
            int cnt = C_h5[i].at(a)[j_starts[i]];
            if (cnt < mn) mn = cnt;
        }
        ub += mn;
    }
    return ub;
}
// ================================== backward BS ==========================================

int h5_backward(
    const Node* node,
    const std::vector<std::unordered_map<char, std::vector<int>>>& C_h5,
    const std::vector<char>& Sigma_h5,
    const std::vector<int>& seq_lens   // not strictly needed, kept for parity
) {
    int m = node.pos.size();
    int h = 0;

    for (char a : Sigma_h5) {
        int min_count = std::numeric_limits<int>::max();

        for (int i = 0; i < m; ++i) {
            int p_i = node->pos[i];

            if (p_i < 0) {
                min_count = 0;
                break;
            }

            // prefix_count = total_count - suffix_count(p_i + 1)
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

// ====================== First valid indices helper ======================
inline std::unordered_map<char, int> first_valid_indices(
    
    const std::string& seq,
    int start_idx,
    const std::vector<int>& gap_arr,
    int prev_idx
) {
    std::unordered_map<char, int> mapping;
    for (size_t j = start_idx; j < seq.size(); ++j) {
        if (prev_idx != -1 && static_cast<int>(j - prev_idx) > gap_arr[j] + 1) {
            continue;
        }
        char c = seq[j];
        if (mapping.find(c) == mapping.end()) {
            mapping[c] = static_cast<int>(j);
        }
    }
    return mapping;
}

 
