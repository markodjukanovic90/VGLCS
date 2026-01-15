#ifndef BEAMSEARCH_H
#define BEAMSEARCH_H

#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <chrono>
#include <unordered_map>

#include "Node.h"
#include "utils.h"
#include "Instance.h"

class BeamSearch {
public:

    struct Result {
        std::string best_seq;
        std::vector<std::vector<int>> steps;
        double runtime;
    };

    static Result run_forward_BS(
        Instance* inst,
        bool forward_or_backward,
        int beam_width = 10,
        HeuristicType heuristic = HeuristicType::H1,  //  
        int time_limit_sec = 1800
    ) {
        const auto& sequences = inst->sequences;
        const auto& gaps = inst->gaps;
        const int m = sequences.size();

        std::vector<Node*> beam;
        std::vector<int> init_pos(m, -1);
        int max_iters = 100000; // should be parametrized 

        Node* start_node = new Node(init_pos, "", nullptr);
        beam.push_back(start_node);

        std::string best_seq;
        Node* return_node = nullptr;

        auto time_start = std::chrono::steady_clock::now();

        for (int iter = 0; iter < max_iters && !beam.empty(); ++iter) {

            if (std::chrono::duration<double>(
                    std::chrono::steady_clock::now() - time_start
                ).count() > time_limit_sec)
                break;

            std::vector<Node*> candidates;

            for (Node* node : beam) 
          {
                auto succs = forward_or_backward
                    ? Node::generateSuccessors(node, inst)
                    : Node::generateBackwardSuccessors(node, inst);

                for (Node* s : succs) {
                    candidates.push_back(s);
                    if (s->seq.size() > best_seq.size()) {
                        best_seq = s->seq;
                        return_node = s;
                    }
                }
            }  

            if (candidates.empty()) break;

            for (Node* n : candidates)
                n->evaluate(inst, heuristic, 0, forward_or_backward);

            std::sort(candidates.begin(), candidates.end(),
                [](Node* a, Node* b) { return a->score > b->score; });

            if ((int)candidates.size() > beam_width)
                candidates.resize(beam_width);

            for (Node* n : beam)
                delete n;

            beam = candidates;
        }
        //constructing solution steps


        std::vector<std::vector<int>> steps;
        if (return_node) {
            for (Node* n = return_node; n->parent; n = n->parent)
                steps.push_back(n->pos);
            std::reverse(steps.begin(), steps.end());
        }

        double runtime = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - time_start
        ).count();

        return { best_seq, steps, runtime };
    }
};

#endif // BEAMSEARCH_H

