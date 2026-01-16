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
        int time_limit_sec = 1800,
        Node* start_node = nullptr
    ) {
        const auto& sequences = inst->sequences;
        const auto& gaps = inst->gaps;
        const int m = sequences.size();
        std::vector<Node*> all_nodes; // trace all nodes, at the end delete them all

        std::vector<Node*> beam;
        std::vector<int> init_pos(m, -1);
        int max_iters = 100000; // should be parametrized 
        
        if(start_node == nullptr)
            start_node = new Node(init_pos, "", nullptr); // start from the beginning

        
        start_node->print();
        beam.push_back(start_node);

        std::string best_seq;
        Node* return_node = nullptr;

        auto time_start = std::chrono::steady_clock::now();

        for (int iter = 0; iter < max_iters && !beam.empty(); ++iter) {
        
            std::cout <<"#iter: " << iter << std::endl;
            
            if (std::chrono::duration<double>(
                    std::chrono::steady_clock::now() - time_start
                ).count() > time_limit_sec)
                break;

            std::vector<Node*> candidates;

            for (Node* node : beam) 
            {
                //node->print();
                //std::cout <<"expand ... " << std::endl;
                auto succs = forward_or_backward
                    ? Node::generateSuccessors(node, inst)  //TODO: @generateSuccessors issue 
                    : Node::generateBackwardSuccessors(node, inst);
                                
                if (succs.size()==0) break;

                for (Node* s : succs) {
                    candidates.push_back(s);
                    all_nodes.push_back(s);
                    if (s->seq.size() > best_seq.size()) {
                        best_seq = s->seq;
                        return_node = s;
                    }
                }
            }  
            
            //std::cout << "\ncandidates: " << (candidates.size());
            if (candidates.empty()) break;

            for (Node* n : candidates)
                n->evaluate(inst, heuristic, 0, forward_or_backward);

            std::sort(candidates.begin(), candidates.end(),
                [](Node* a, Node* b) { return a->score > b->score; });

            if ((int)candidates.size() > beam_width)
                candidates.resize(beam_width);
                
            beam = candidates;
        }
        //constructing solution steps
	    std::vector<std::vector<int>> steps;

	    if (return_node) {
    		return_node->print();

    	    while (return_node != nullptr) 
            {
                 if (return_node->pos[0] != -1)
        	         steps.push_back(return_node->pos);
                 return_node = return_node->parent;
            }   

    	    std::reverse(steps.begin(), steps.end());
        }


        double runtime = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - time_start
        ).count();
        
        // clean up the memory(release the occupied memory)
        for (Node* n : all_nodes)
            delete n;

        return { best_seq, steps, runtime };
    }
};

#endif // BEAMSEARCH_H

