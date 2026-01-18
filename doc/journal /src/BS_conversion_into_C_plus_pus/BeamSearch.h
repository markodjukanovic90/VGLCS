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
        std::vector<std::vector<int>> list_pos_complete;
    };
    
    struct VectorHash {
      std::size_t operator()(const std::vector<int>& v) const noexcept {
        std::size_t h = 0;
        for (int x : v) {
            h ^= std::hash<int>{}(x) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
      }
   };




    static Result run_forward_backward_BS(
        Instance* inst,
        bool forward_or_backward,
        int beam_width = 10,
        HeuristicType heuristic = HeuristicType::H1,  //  
        int time_limit_sec = 1800,
        const std::vector<Node*>& start_node_vector = {}
    ) {
        const auto& sequences = inst->sequences;
        const auto& gaps = inst->gaps;
        const int m = sequences.size();
        std::vector<Node*> all_nodes; // trace all nodes, at the end delete them all
        
        std::vector<std::vector<int>> list_pos_complete;
        std::vector<Node*> beam;
        std::vector<int> init_pos(m, -1);
        int max_iters = 100000; // should be parametrized 
        
        
        Node* start_node = nullptr;
        //if(start_node == nullptr)
        if(start_node_vector.size() == 0 ){
            start_node = new Node(init_pos, "", nullptr); // start from the beginning
            beam.push_back(start_node);
        }
        else{
             for(Node* n: start_node_vector)
                 beam.push_back(n);
        }
        //start_node->print(); //std::cout << "beam_width: " << beam_width << std::endl;
        //beam.push_back(start_node);

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
                
                if (succs.size()==0){ 
                    list_pos_complete.push_back(nx->pos);
                    continue; 
                }

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
            
            if (forward_or_backward) // if not backward, no need to reverse back the @str value, only the collected steps
    	        std::reverse(steps.begin(), steps.end());
    	    else
    	        std::reverse(best_seq.begin(), best_seq.end()); // other
        }


        double runtime = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - time_start
        ).count();
        
        // clean up the memory(release the occupied memory)
        for (Node* n : all_nodes)
            delete n;

        return { best_seq, steps, runtime, list_pos_complete };
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
        
        
        //best solution 
        int best_sol_found=0; std::string best_seq="";
        std::vector<Node*> R; // can be also pririty queue: TODO 
        R.push_back(new Node(std::vector<int>( inst->sequences.size(), -1), "", nullptr) );
        std::unordered_map<std::vector<int>, Node*, VectorHash> visited;
        
        for(int iter=0; iter < imsbs_iterations; ++iter)
        {    
             std::cout << "#iter=" << iter << std::endl;
             int r_size =   std::min(static_cast<size_t>(number_root_nodes), R.size());
             
             std::vector<Node*> L;
             while (L.size() < r_size) 
             {
                  L.push_back(R[0]);
                  R.erase(R.begin());
             }             
             //L initialized; refine the nodes by running backward BS per each node 
             for(Node *n : L)
             {
                 
   	           Result res_n = run_forward_backward_BS(
        		inst,
        		false, // backward BS
        		beam_width_backward,
        		heuristic_backward,
        		time_limit_sec, 
        		{n} // run the backward BS on @n
    		  );
    		  //update the node n  (refine it)
    		  n->seq = res_n.best_seq;
    		  //n->pos = res_n.steps; // this about how to reconstruct (pass) the final steps (pairs of matchings by obtainined the solution)
    		  n->parent = nullptr;  
             } 
             //execute the FORWARD BS on the set of refined nodes from L:
              Result res_n = run_forward_backward_BS(
        			inst,
        			true, // backward BS
        			beam_width_forward,
        			heuristic_forward,
        			time_limit_sec, 
        			L // run the forward BS on L
             );
             //   check for a new incumbent solution 
             if(best_sol_found < res_n.best_seq.size())
             {
                 best_sol_found=res_n.best_seq.size();
                 best_seq =  res_n.best_seq;
                 //reconstruct steps from the backward BS: TODO 
             }
             // Update R: should pass all complete solutions during the forward BS
             for(auto& pos: res_n.list_pos_complete)
             {
                 Node* p_new = new Node(pos, "", nullptr); // create a new node as a candidate root node
                 // expand p_new 
                 auto succs = Node::generateSuccessorsLCS(p_new, inst, heuristic_backward); // generate in the LCS manner 
                 // add succs into R if not already been part of R (in the previous iterations)
                 for(Node* child: succs){
                     if(visited.find(child->pos) == visited.end()) // @child not in @visited
                     {
                         R.push_back(child);
                         visited.emplace(child->pos, child);
                      }
                 }
             }
        }
        
        return {};
    }
};

#endif // BEAMSEARCH_H

