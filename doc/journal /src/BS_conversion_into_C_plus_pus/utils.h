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
    const std::vector<int>& node_pos,
    const std::vector<std::unordered_map<char, std::vector<int>>>& C_h5,
    const std::vector<char>& Sigma_h5,
    const std::vector<int>& seq_lens
) {
    int m = static_cast<int>(C_h5.size());
    std::vector<int> j_starts(m);
    for (int i = 0; i < m; ++i) {
        j_starts[i] = std::min(node_pos[i] + 1, seq_lens[i]);
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

// ====================== Generate successors ======================
inline std::vector<Node> generate_successors(
    const Node& node,
    const std::vector<std::string>& sequences,
    const std::vector<std::vector<int>>& gaps
) {
    int m = static_cast<int>(sequences.size());

    std::vector<std::unordered_map<char,int>> per_seq_maps(m);
    for (int i = 0; i < m; ++i) {
        per_seq_maps[i] = first_valid_indices(sequences[i], node.pos[i] + 1, gaps[i], node.pos[i]);
    }

    // Find common characters across sequences
    std::unordered_set<char> common_chars;
    for (const auto& kv : per_seq_maps[0]) common_chars.insert(kv.first);
    for (int i = 1; i < m; ++i) {
        std::unordered_set<char> tmp;
        for (const auto& kv : per_seq_maps[i]) tmp.insert(kv.first);
        std::unordered_set<char> intersect;
        for (char c : common_chars) if (tmp.count(c)) intersect.insert(c);
        common_chars = intersect;
    }

    if (common_chars.empty()) return {};

    // Remove dominated characters
    std::unordered_set<char> dom;
    for (char c : common_chars) {
        for (char o : common_chars) {
            if (c == o) continue;
            bool dominates = true;
            for (int i = 0; i < m; ++i) {
                int ci = per_seq_maps[i].at(c);
                int oi = per_seq_maps[i].at(o);
                if (!(ci <= oi && gaps[i][oi] + ci + 1 >= oi)) {
                    dominates = false;
                    break;
                }
            }
            if (dominates) dom.insert(o);
        }
    }
    for (char d : dom) common_chars.erase(d);

    // Build successors
    std::vector<Node> successors;
    std::vector<char> sorted_chars(common_chars.begin(), common_chars.end());
    std::sort(sorted_chars.begin(), sorted_chars.end());

    for (char ch : sorted_chars) {
        std::vector<int> idx_vector(m);
        std::vector<int> jumps(m);
        for (int i = 0; i < m; ++i) {
            idx_vector[i] = per_seq_maps[i].at(ch);
            jumps[i] = node.pos[i] == -1 ? 0 : idx_vector[i] - node.pos[i];
        }
        successors.emplace_back(idx_vector, node.seq + ch, node.parent);
    }

    return successors;
}

#endif // UTILS_H

