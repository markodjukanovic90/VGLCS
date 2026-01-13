#include "beam_search.h"
#include "Node.h"
#include "globals.h"
#include <assert.h>
#include <Eigen/Dense>
#include "unistd.h"

using namespace std;

 
// global vars and data structures
double running_time = 0.0;
std::vector<int> solution;

bool validate_solution(Instance* inst)
{

    if(solution.size() == 0)
        return false;

    for(int i=0; i < inst->m; ++i)
    {
         int count = 0;
         for(auto& si: inst->S[i])
         {
             if(si == solution[count])
                 count++;
             if(count == (int)solution.size())
                break; // move to the next string si
         }
         if(count < (int)solution.size())
         {
             std::cout << "string " << i << " is not a supersequence of the solution "
                       <<  count << " and needs to be " << solution.size() <<  std::endl;
             return false;
         }
    }
    // P-string inclusion
    for(int j=0; j < inst->p; ++j)
    {
        int count = 0;
        for(auto& s: solution)
        {
            if(inst->P[j][count] == s)
                count++;
            if(count == (int)inst->P[j].size())
               break;
        }
        if(count < (int)inst->P[j].size())
            return false;
    }
    // R-strings -- non-inclusion
    for(int k=0; k < inst->r; ++k)
    {
        int count = 0;
        for(auto& s: solution)
        {
            if(s == inst->R[k][count])
                count++;
            if(count == (int)inst->R[k].size())
                return false;
        }
   }    
   return true;
}

void save_in_file(const std::string& outfile, Instance* inst)
{
    //clean filename
    string clean_file_name = "";
    int bar_count = 0;
    int count = 0;
    for(char c : filename) if(c == '/') bar_count ++;
    for(char c : filename){
        if(count == bar_count and c == '.') break;
        if(count == bar_count) clean_file_name.push_back(c);
        else if(c == '/') count ++;
    }

    if(outfile==""){
        std::cout << clean_file_name << std::endl;
        std::cout << "Objective: " << solution.size() << std::endl;
        std::cout << "Solution: ";
        for(auto& c: solution)
            std::cout << inst->int2char[c] << " " ;

        std::cout << "\nTime: " << running_time << std::endl;
        std::cout << "Feasible: " << validate_solution(inst) << std::endl;
        return;
    }

     std::ofstream outputFile(outfile);
     // Check if the file is opened successfully
     if (!outputFile.is_open()) {
          std::cerr << "Error opening the file." << std::endl;
          return; // Return an error code
     }
     // write in an out-file
     outputFile << clean_file_name << std::endl;
     outputFile << "Objective: " << solution.size() << std::endl;
     outputFile << "Solution: ";
     for(auto& c: solution)
         outputFile << inst->int2char[c] << " " ;

     outputFile << "\nTime: " << running_time << std::endl;
     outputFile << "Feasible: " << validate_solution(inst) << std::endl;
     outputFile.close();
}

// Feature calculation:

double compute_max(const vector<double>& values){
    auto max_element = std::max_element(values.begin(), values.end());
    return *max_element;
}

double compute_min(const vector<double>& values){
    auto min_element = std::min_element(values.begin(), values.end());
    return *min_element;
}

double compute_average(const vector<double>& values){
    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    return sum / values.size();
}

double compute_std(const vector<double>& values, double mean){
    double squaredDifferencesSum = 0.0;
    for (double value : values) {
        squaredDifferencesSum += (value - mean) * (value - mean);        
    }
    double variance = squaredDifferencesSum / values.size();
    return std::sqrt(variance);
}

void standardize(vector<double>& features){

    double features_avg = compute_average(features);
    double features_std = compute_std(features, features_avg);
    
    for(double& feature : features)
        feature = (feature - features_avg) / features_std;
}

void compute_features(std::vector<Node*>& V_ext){
    //calculate features for the nodes in V_ext
    for(Node* node : V_ext){
        // first option: consider p^{L,v} and l^v. first do p^{L,v}_i = p^{L,v}_i / |s_i| and l^v_j = l^v_j / |r_j| for al input and restricted strings. 
        // then consider the max, min, std, avg of both vectors as features (8 features). Add also length of partial sol represented by node u^v (9 in total) (config1)
        // Add the alphabet size (config2)
        // Add length and number of input and restricted strings (config3)
        vector<double> pL_v(get<0>(node->position).begin(), get<0>(node->position).end());
        vector<double> lv(get<2>(node->position).begin(), get<2>(node->position).end());
        //normalize left position vectors so that they do not depend on the length of the input and restricted strings
        for(int i = 0; i < (int)pL_v.size(); ++i) pL_v[i] = pL_v[i] / node->inst->S[i].size();
        for(int i = 0; i < (int)lv.size(); ++i) lv[i] = lv[i] / node->inst->R[i].size();
        
        vector<double> features; 
        //features of vectors pL_v and lv
        double mean_pL_v = compute_average(pL_v);
        double mean_lv = compute_average(lv);
        features.push_back(compute_max(pL_v));
        features.push_back(compute_min(pL_v));
        features.push_back(mean_pL_v);
        features.push_back(compute_std(pL_v, mean_pL_v));
        features.push_back(compute_max(lv));
        features.push_back(compute_min(lv));
        features.push_back(mean_lv);
        features.push_back(compute_std(lv, mean_lv));
        features.push_back(node->l_v);
        
        if(feature_config == 2) //add alphabet size
            features.push_back(node->inst->Sigma);
        else if(feature_config == 3){ //add number of input and restricted strings
            features.push_back(node->inst->Sigma);
            features.push_back(node->inst->m);
            features.push_back(node->inst->r);
        }
        else if(feature_config == 4){ //add length of input and restricted strings
            features.push_back(node->inst->Sigma);
            features.push_back(node->inst->m);
            features.push_back(node->inst->r);
            features.push_back((node->inst->S[0]).size()); //note that this config only makes sense if all input strings have the same length
            features.push_back((node->inst->R[0]).size()); //and the same happens with the restricted strings 
        }
        standardize(features);
        node->features = features;
    }
}

void compute_heuristic_values(std::vector<Node*>& V_ext, MLP& neural_network){ //add the option of using the probabilistic guiding function  
    for(Node* node : V_ext){
    
        Eigen::Map<const Eigen::VectorXd> eigen_features(node->features.data(), node->features.size());
        node->heuristic_value = neural_network.forward(eigen_features)(0);
    }
}

double BS(double t_lim, int beta, Instance* inst, MLP& neural_network, bool training)
{

    // Create the root node r and add it to the @beam
    std::vector<int> pL(inst->m, 0);
    std::vector<int> ppl(inst->p, 0);
    std::vector<int> rpl(inst->r, 0);
    Node *root = new Node(inst, pL, ppl, rpl);
    // create a beam, initialited with the root node
    std::vector<Node*> to_delete; // Nodes to be deleted after the execution of BS
    std::vector<Node*> beam;
    beam.push_back(root);
    to_delete.push_back(root);
    int beam_size = beta;
    int l_best = 0;
    double time_monitoring = 0.0;
    Node* best_node = root;
    // instantitate the clock 
    auto start_time = std::chrono::high_resolution_clock::now();
    std::set<std::tuple<std::vector<int>, std::vector<int>>> node_in_set; // remove duplicate nodes before storing in @beam
    
    while(!beam.empty()) {

        std::vector<Node*> V_ext;
        for(const auto & node: beam) {  
            
            std::vector<Node*> children_nodes = node->expansion(); 
            
            if(children_nodes.empty()) {
                if(node->l_v > l_best && node->is_complete()) { // update the best solution (node has no children) 
                    l_best = node->l_v;
                    best_node = node;
                }
            }
            // otherwise add those nodes in V_ext set (remove duplicated ones)
            for(auto& child : children_nodes) { // add into V_ext (if not already in the set)
                if (node_in_set.find(std::make_tuple(std::get<0>(child->position), std::get<2>(child->position))) == node_in_set.end()) {    
                    V_ext.push_back(child);    
                    node_in_set.insert(std::make_tuple(std::get<0>(child->position), std::get<2>(child->position)));
                } 
                else { 
                    delete child;
                    continue;
                }
            }
        }
         // Use the neural network for sorting V_ext: First to calculate features and then call the neural network for every node in V_ext
         compute_features(V_ext);
         compute_heuristic_values(V_ext, neural_network);
         std::sort(V_ext.begin(), V_ext.end(), [](Node* const& n1, Node* const& n2){return n1->heuristic_value > n2->heuristic_value;});

         // look for ties and calculate the secondary heurstic value for the nodes that are tied   
         if(differentiate_ties){
            if((int)V_ext.size() > beam_size){ //look for ties
                double limit_h = V_ext[beam_size - 1]->heuristic_value;
                if(limit_h == V_ext[beam_size]->heuristic_value){ //there are ties
                    int backwards_count = 0;
                    while(V_ext[beam_size - 1 - backwards_count - 1]->heuristic_value == limit_h){
                        backwards_count++;
                    }
                    int forwards_count = 1;
                    while(V_ext[beam_size - 1 + forwards_count + 1]->heuristic_value == limit_h){
                        forwards_count ++;
                    }
                    vector<Node*> tied_V_ext(V_ext.begin() + beam_size - 1 - backwards_count, V_ext.begin() + beam_size - 1 + forwards_count + 1);

                    //calculate secondary_heuristic_value for the nodes in tied_V_ext
                    int k = numeric_limits<int>::max();
                    for(auto& v : tied_V_ext){   
                        int index=0;
                        std::vector<int> pos = std::get<0>(v->position);

                        for(int& pli: pos){
                            if(k > (int)inst->S[index].size() - pli + 1)
                            k=inst->S[index].size() - pli + 1;
                            index++; 
                        }  
                    }
                    if( double(k) / inst->Sigma < 1.0 ) k = 1;  
                    else k = std::ceil( double(k) / inst->Sigma);

                    for(auto& v: tied_V_ext) 
                        v -> set_up_prob_value(k);
                    
                    std::sort(tied_V_ext.begin(), tied_V_ext.end(), [](Node* const& n1, Node* const n2){return n1->secondary_heuristic_value > n2->secondary_heuristic_value;});
                    //include tied_V_ext back into V_ext but now sorted.
                    std::copy(tied_V_ext.begin(), tied_V_ext.end(), V_ext.begin() + beam_size - 1 - backwards_count);
                }
            }
        }  
        // the last phase: clean the old beam, replace it with a new best beta nodes fron V_ext
        beam.clear();
        int num_nodes_in_beam  = 0;
        for(auto const &v : V_ext)
        {  
            if(num_nodes_in_beam < beam_size){
                beam.push_back(v);
                to_delete.push_back(v);
                num_nodes_in_beam++;
            }
            else delete v;
        }
        node_in_set.clear();
        // time limit monitoring:
        auto end_time = std::chrono::high_resolution_clock::now();
        // Calculate the duration in microseconds
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        time_monitoring = double(duration.count()) / 1000;
        if(t_lim < time_monitoring)
             break;
    }
    running_time = time_monitoring;
    solution = best_node->export_solution();
    save_in_file(outpathname, inst);
 
    for(Node* node : to_delete) delete node; //free up the memory 
    
    return l_best;
}

