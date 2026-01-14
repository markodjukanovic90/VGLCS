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
    // Generate child nodes (ekvivalent generate_successors)
    // ============================================================
    static std::vector<Node*> generateSuccessors(
        Node* node,
        Instance* inst
    ) {
        const std::vector<std::string>& sequences = inst->sequences;
        const std::vector<std::vector<int>>& gaps = inst->gaps;
        const int m = sequences.size();

        // per_seq_maps[i] : map<char, index>
        std::vector<std::unordered_map<char, int>> perSeqMaps(m);

        for (int i = 0; i < m; ++i) {
            std::unordered_map<char, int> map;
            for(char ch: inst->Sigma) {
                map[ch] = inst->Succ[i][ch][node->pos[i] + 1];
            }   
            perSeqMaps[i] = map;            

            //firstValidIndices(
            //    sequences[i],
            //    node->pos[i] + 1,
            //    gaps[i],
            //    node->pos[i]
            //
        }
        
        // --------------------------------------------------------
        // Presjek karaktera koje sve sekvence mogu mečovati
        // --------------------------------------------------------
        std::set<char> commonChars;
        bool first = true;
        
        for(char ch: inst->Sigma) {
                // pitaj da li se ch nalazi u svim mapama (index i odozgo) i da je različit od -1
                bool found_in_all = true;
                for (int i = 0; i < m; ++i) {  
                    if (perSeqMaps[i].find(ch) == perSeqMaps[i].end() ||
                        perSeqMaps[i][ch] == -1) {
                        found_in_all = false;
                        break;
                    }
                }
                if (found_in_all) {
                    commonChars.insert(ch);
                }
        }
        

        if (commonChars.empty()) // this is the complete node 
            return {};

        // --------------------------------------------------------
        // Removal of dominated characters from @commonChars
        // --------------------------------------------------------

        std::set<char> dominated;

        for (char c : commonChars) {
            for (char o : commonChars) { //run through the pairs of different characters (c, o)
                if (c == o) continue;

                bool dominates = true;
                for (int i = 0; i < m; ++i) {
                    int dc = perSeqMaps[i].at(c);
                    int do_ = perSeqMaps[i].at(o);
                    if (!(dc <= do_ &&
                          gaps[i][do_] + dc + 1 >= do_)) {  // breaking symmetric condition
                        dominates = false;
                        break;
                    }
                }
                if (dominates)
                    dominated.insert(o);
            }
        }

        for (char d : dominated) // remove dominated chars from commonChars
            commonChars.erase(d);

        // --------------------------------------------------------
        // Kreate new (child) nodes 
        // --------------------------------------------------------
        std::vector<Node*> successors;

        for (char ch : commonChars) {
            std::vector<int> idxVector(m);

            for (int i = 0; i < m; ++i)
                idxVector[i] = perSeqMaps[i].at(ch); // new vector of positions 

            Node* child = new Node(
                idxVector,
                node->seq + ch,// to optimize
                node
            );

            successors.push_back(child);
        }

        return successors;
    }
    
// Generate successors in BACKWARD manner
// (equivalent to generate_backward_successors in Python)
// ============================================================

static std::vector<Node*> generateBackwardSuccessors(
    Node* node, Instance* inst,
) {
    const std::vector<std::string>& sequences = inst->sequences;
    const std::vector<std::vector<int>>& gaps = inst->gaps;
    const std::vector<std::unordered_map<char, std::vector<int>>>& Prev   =   inst->Prev;
    const std::set<char>& Sigma = inst->Sigma;

    const int m = sequences.size();
    std::vector<Node*> successors;

    for (char a : Sigma) {
    
        std::vector<int> prev_positions;
        bool feasible = true;

        for (int i = 0; i < m; ++i) {
            int pi = node->pos[i];

            if (pi < 0) {
                feasible = false;
                break;
            }

            // Prev[i][a][pi]
            auto it = Prev[i].find(a);
            // if not found or index is -1, not feasible   
            if (it == Prev[i].end() || *it->second == -1) { 
                feasible = false;
                break;
            }
            //otherwise retrive the prev index
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

            // gap constraint fulfillment 
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
            score = remaining_lb(this->pos, inst->sequences); 
            break;

        case HeuristicType::H5:
            if (forward_or_backward) // forward BS
                score = h5_upper_bound(this->pos, inst->C_suffix,
                                   inst->Sigma,
                                   inst->sequences);     
            else
                score = h5_backward(this, inst->C_suffix,
                                   inst->Sigma);  
            break; 
              


        case HeuristicType::H8:  
            score =  probability_based_heuristic(this->pos, inst->P, inst->sequences, k);  
            break;
    }
}

private:
    // ============================================================
    // Helper: _first_valid_indices (maybe can be removed)
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

