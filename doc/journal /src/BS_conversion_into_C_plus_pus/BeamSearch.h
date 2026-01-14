#ifndef BEAMSEARCH_H
#define BEAMSEARCH_H

#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <chrono>
#include "Node.h"
#include "Utils.h"

class BeamSearch {

    bool forward_or_backward = true;

    ) {
public:

    struct Result {
        std::string best_seq;
        std::vector<std::vector<int>> steps;
        double runtime; // seconds
    };

    static Result run(

        Instance* inst,
        int beam_width = 10,
        HeuristicType heuristic = HeuristicType::H1,
        int max_iters = 10000,
        int time_limit_sec = 1800
    ) {
        const std::vector<std::string>& sequences = inst->sequences;
        const std::vector<std::vector<int>>& gaps = inst->gaps;
        const int m = sequences.size();

        // ---------- početno stanje ----------
        std::vector<Node*> beam;
        std::vector<int> init_pos(m, -1);

        Node* start_node = new Node(init_pos, "", nullptr);
        beam.push_back(start_node);  

        std::string best_seq = "";
        Node* return_node = nullptr;

        // h5 / h8 priprema
        std::vector<char> Sigma_h5 = inst->Sigma;
        std::vector<std::unordered_map<char, std::vector<int>>> C_h5 = inst->C_suffix;

        auto time_start = std::chrono::steady_clock::now(); // start time

        for (int iter = 0; iter < max_iters; ++iter)
        {  
            if (beam.empty()) break;

            // ---------- Time limit ----------
            auto time_now = std::chrono::steady_clock::now();
            auto elapsed_sec = std::chrono::duration<double>(time_now - time_start).count();

            if (elapsed_sec > time_limit_sec) break;

            // ---------- Expansion of beam -----
            std::vector<Node*> candidates;

            for (Node* node : beam) {   
                std::vector<Node*> succs;

                if (forward_or_backward == false) { // backward BS
                    succs = Node::generateBackwardSuccessors(node, inst);
                } else {   
                    succs = Node::generateSuccessors(node, sequences, gaps);  
                }
                //Child nodes of the @node 
                for (Node* s : succs) {
                    candidates.push_back(s);
                    if (static_cast<int>(s->seq.size()) > static_cast<int>(best_seq.size())) {
                        best_seq = s->seq;
                        return_node = s;
                    }
                }
            }
            // complete node 
            if (candidates.empty()) break;

            // ---------- Ranking and shrinking the Candidates to create new beam, first node evaluation  ----------
            for (Node* n : candidates) {
                n->evaluate(nullptr, static_cast<int>(heuristic), 0, heuristic != HeuristicType::H1);
            }

            std::sort(candidates.begin(), candidates.end(),
                [](Node* a, Node* b) { return a->score > b->score; });

            if (static_cast<int>(candidates.size()) > beam_width)
                candidates.resize(beam_width);

            // Beam replacement 
            for (Node* n : beam) delete n; // cleanup old beam   
            beam = candidates;  
        }

        // ---------- Path reconstruction ----------
        std::vector<std::vector<int>> steps;
        if (return_node != nullptr) {
            Node* n = return_node;
            best_seq = n->seq;
            while (n->parent != nullptr) {
                steps.push_back(n->pos);
                n = n->parent;
            }
            std::reverse(steps.begin(), steps.end());
        }

        auto time_end = std::chrono::steady_clock::now();
        double runtime_sec = std::chrono::duration<double>(time_end - time_start).count();

        // cleanup preostalih node-ova
        for (Node* n : beam) delete n;

        return { best_seq, steps, runtime_sec };
    }

private:
    // TODO: implementirati h5 C_h5 pripremu
    static std::vector<std::unordered_map<char, std::vector<int>>> build_suffix_counts(
        const std::vector<std::string>& sequences,
        const std::vector<char>& Sigma
    ) {
        // placeholder, implementirati prema tvojoj Python funkciji
        return {};
    }
};

#endif // BEAMSEARCH_H

