#ifndef NODE_H
#define NODE_H

#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <algorithm>
#include "Instance.h"
#include "utils.h"

class Node {
public:
    // Indeksi zadnjeg meča za svaku sekvencu (-1 = prije početka)
    std::vector<int> pos;

    // Trenutno izgrađeni LCS (ako se ne rekonstruiše preko parent-a)
    std::string seq;

    // Pokazivač na roditeljski čvor
    Node* parent;

    double score;        // vrijednost heuristike 

    // Konstruktor
    Node(const std::vector<int>& pos_,
         const std::string& seq_,
         Node* parent_ = nullptr)
        : pos(pos_), seq(seq_), parent(parent_) {}

    // Ekvivalent @property length
    int length() const {
        return static_cast<int>(seq.size());
    }

    // ============================================================
    // Generisanje nasljednika (ekvivalent generate_successors)
    // ============================================================
    static std::vector<Node*> generateSuccessors(
        Node* node,
        const std::vector<std::string>& sequences,
        const std::vector<std::vector<int>>& gaps
    ) {
        const int m = sequences.size();

        // per_seq_maps[i] : map<char, index>
        std::vector<std::unordered_map<char, int>> perSeqMaps(m);

        for (int i = 0; i < m; ++i) {
            perSeqMaps[i] = firstValidIndices(
                sequences[i],
                node->pos[i] + 1,
                gaps[i],
                node->pos[i]
            );
        }

        // --------------------------------------------------------
        // Presjek karaktera koje sve sekvence mogu mečovati
        // --------------------------------------------------------
        std::set<char> commonChars;
        bool first = true;

        for (const auto& mp : perSeqMaps) {
            std::set<char> chars;
            for (const auto& kv : mp) chars.insert(kv.first);

            if (first) {
                commonChars = chars;
                first = false;
            } else {
                std::set<char> tmp;
                std::set_intersection(
                    commonChars.begin(), commonChars.end(),
                    chars.begin(), chars.end(),
                    std::inserter(tmp, tmp.begin())
                );
                commonChars.swap(tmp);
            }
        }

        if (commonChars.empty())
            return {};

        // --------------------------------------------------------
        // Uklanjanje dominirajućih simbola
        // --------------------------------------------------------
        std::set<char> dominated;

        for (char c : commonChars) {
            for (char o : commonChars) {
                if (c == o) continue;

                bool dominates = true;
                for (int i = 0; i < m; ++i) {
                    int dc = perSeqMaps[i].at(c);
                    int do_ = perSeqMaps[i].at(o);
                    if (!(dc <= do_ &&
                          gaps[i][do_] + dc + 1 >= do_)) {
                        dominates = false;
                        break;
                    }
                }
                if (dominates)
                    dominated.insert(o);
            }
        }

        for (char d : dominated)
            commonChars.erase(d);

        // --------------------------------------------------------
        // Kreiranje novih Node-ova
        // --------------------------------------------------------
        std::vector<Node*> successors;

        for (char ch : commonChars) {
            std::vector<int> idxVector(m);

            for (int i = 0; i < m; ++i)
                idxVector[i] = perSeqMaps[i].at(ch);

            Node* child = new Node(
                idxVector,
                node->seq + ch,
                node
            );

            successors.push_back(child);
        }

        return successors;
    }
    
    // =====================GENERATE NODES IN BACKWARD MANNER ======================================= 
    // ============================================================
// Generate successors in BACKWARD manner
// (equivalent to generate_backward_successors in Python)
// ============================================================
static std::vector<Node*> generateBackwardSuccessors(
    Node* node, Instace* inst,
) {
    const std::vector<std::string>& sequences = inst->sequences;
    const std::vector<std::vector<int>>& gaps = inst->gaps;
    const std::vector<std::unordered_map<char, std::vector<int>>>& Prev =   inst->Prev;
    const std::set<char>& Sigma = inst->Sigma;

    const int m = sequences.size();
    // p-vector (copy current positions)
    std::vector<int> p = node->pos;

    std::vector<Node*> successors;

    for (char a : Sigma) {
        std::vector<int> prev_positions;
        bool feasible = true;

        for (int i = 0; i < m; ++i) {
            int pi = p[i];

            if (pi < 0) {
                feasible = false;
                break;
            }

            // Prev[i][a][pi]
            auto it = Prev[i].find(a);
            if (it == Prev[i].end()) {
                feasible = false;
                break;
            }

            const std::vector<int>& prev_vec = it->second;

            if (pi >= static_cast<int>(prev_vec.size())) {
                feasible = false;
                break;
            }

            int pa = prev_vec[pi];

            if (pa == -1) {
                feasible = false;
                break;
            }

            // gap constraint
            if (pi - pa - 1 > gaps[i][pi]) {
                feasible = false;
                break;
            }

            prev_positions.push_back(pa);
        }

        if (!feasible)
            continue;

        // build child
        Node* child = new Node(
            prev_positions,
            node->seq + a,   // backward → append
            node
        );

        successors.push_back(child);
    }

    return successors;
}
    // ============================================================

    void evaluate(Instance* inst, int heuristic, int k = 0, bool forward_or_backward = true) { // required to pass for forward and backward BS

    switch (heuristic) {

        case HeuristicType::H1:
            score = this->length(); //lv  
            break;

        case HeuristicType::H2:
            score = remaining_lb(this->pos, inst->sequences); // TODO  
            break;

        case HeuristicType::H5:
            if (forward_or_backward) // forward BS
                score = h5_upper_bound(this->pos, inst->C_suffix,
                                   inst->Sigma,
                                   inst->sequences);     
            else
                score = h5_backward(this, inst->C_suffix,
                                   inst->Sigma); // TODO 
            break; 
              


        case HeuristicType::H8:  
            score =  probability_based_heuristic(this->pos, inst->P, inst->sequences, k); // TODO
            break;
    }
}

private:
    // ============================================================
    // Helper: _first_valid_indices
    // ============================================================
    static std::unordered_map<char, int> firstValidIndices(
        const std::string& seq,
        int startIdx,
        const std::vector<int>& gapArr,
        int prevIdx
    ) {
        std::unordered_map<char, int> mapping;

        for (int j = startIdx; j < static_cast<int>(seq.size()); ++j) {
            if (prevIdx != -1 && (j - prevIdx) > gapArr[j] + 1)
                continue;

            char c = seq[j];
            if (mapping.find(c) == mapping.end()) {
                mapping[c] = j; // prvi (najlijevlji) hit
            }
        }
        return mapping;
    }
};

#endif // NODE_H

