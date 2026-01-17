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



    static Result run_forward_backward_BS(
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
        
        start_node->print(); //std::cout << "beam_width: " << beam_width << std::endl;
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

            for (Node* nx : beam) 
            {

                auto succs = forward_or_backward
                    ? Node::generateSuccessors(nx, inst)    
                    : Node::generateBackwardSuccessors(nx, inst);
                
                if (succs.size()==0) continue;

                for (Node* s : succs) {
                    
                    candidates.push_back(s);
                    all_nodes.push_back(s);
                    
                    if (s->seq.size() > best_seq.size()) {
                        best_seq = s->seq;
                        return_node = s;
                    }
                }
            }  
            //std::cout << "candidates ... " << candidates.size() << std::endl;
            if (candidates.empty()) break;

            for (Node* n : candidates){
                double res_score = n->evaluate(inst, heuristic, 0, forward_or_backward);     
            }
            
            std::sort(candidates.begin(), candidates.end(),
                [](Node* a, Node* b) { return a->score > b->score; });

            if ((int)candidates.size() > beam_width)
                candidates.resize(beam_width);
            
            beam = candidates;
            //std::cout << "New beam ... " << beam.size() << std::endl;
            //for(Node* x: beam)
            //    std::cout << x->pos << std::endl;
        }
        //constructing solution steps
	std::vector<std::vector<int>> steps;

	if (return_node != nullptr) {
    		return_node->print();

    	    while (return_node != nullptr) 
            {
                 if (return_node->pos[0] != -1)
        	      steps.push_back(return_node->pos);
                 return_node = return_node->parent;
            } 
            if (forward_or_backward) // if backward, no need to reverse back
    	        std::reverse(steps.begin(), steps.end());
    	    else
    	        std::reverse(best_seq.begin(), best_seq.end());
        }


        double runtime = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - time_start
        ).count();
        
        // clean up the memory(release the occupied memory)
        for (Node* n : all_nodes)
            delete n;

        return { best_seq, steps, runtime };
    }
    
    // IMSBS algorithm
    static Result imsbs(
        Instance* inst,
        int beam_width_forward = 10,
        int beam_width_backward = 10,
        HeuristicType heuristic_forward = HeuristicType::H1,  
        HeuristicType heuristic_backward = HeuristicType::H5,
        int number_root_nodes = 10,
        int imsbs_iterations = 10000,
        int time_limit_sec = 1800) {
        const auto& sequences = inst->sequences;
        const auto& gaps = inst->gaps;
        const int m = sequences.size();
        std::vector<Node*> all_nodes; // trace all nodes, at the end delete them all

        std::vector<Node*> R; // can be also pririty queue: TODO 
        R.push_back(new Node(std::vector<int>( inst->sequences.size(), -1), "", nullptr) );
        
        for(int i=0; i < imsbs_iterations; ++i)
        {
             std::vector<Node*> L;
             while (L.size() < std::min(static_cast<size_t>(number_root_nodes), R.size())) 
             {
                  L.push_back(R[0]);
                  R.erase(R.begin());
             }             
             //TODO: shall be proceeded
        
        }
        
        return {};
    }
};

#endif // BEAMSEARCH_H

