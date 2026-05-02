#include "Algorithm.h"
#include <iostream>
#include <unordered_map>
#include <map>
#include <queue>
#include <chrono> // measure a time of algorithms execution
#include <algorithm> // for std::sort
#include<iterator> // for back_inserter  
#include <fstream>
#include <bits/stdc++.h>
#include "Node.h"
#include "global_variable.h"  

using astar_node = std::pair<rlcs_position*, int>;


// the class for hashing the values (N-list for A*)
class Hash {
public:
    template <typename T>
    size_t operator()(const T &V) const {

        int hash = std::get<0>(V).size();
        auto vec1 = std::get<0>(V);

        for(int &i : vec1) {
            hash ^= i + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        
        auto vec2 = std::get<1>(V);
        for(int &i : vec2) {
            hash ^= i + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }     
        auto vec3 = std::get<2>(V);
        for(int &i : vec3) {
            hash ^= i + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }    

        return hash;
    }
};


// Define a comparison function for Node objects
struct NodeComparator {
    // Define the comparison operator
    bool operator()(const Node* lhs, const Node* rhs) const {
        // Customize the comparison logic based on your requirements
        if(lhs->f_value == rhs->f_value) 
            return lhs->l_v < rhs->l_v;  // TODO: experimet with additional criterion like average of covered letters from R --> the minumum is prefered...
        else
            return lhs->f_value < rhs->f_value; // For a min-heap, use '>'; for a max-heap, use '<'
    }
};

/**  end of helper structures declaration for A* search **/
 
Algorithm::Algorithm(int alg, Instance* instance, double t_lim, int beta)
{
    
    this->inst = instance;
    this->number_algorithm = alg;
    this->timelimit = t_lim;
    this->beta = beta;
}

void Algorithm::run()
{

    switch(this->number_algorithm)
    {
       case 0: Greedy(this->timelimit, 1); break;  
       case 1: BS(this->timelimit, this->beta); break;
       case 2: A_star(this->timelimit); break;
       case 3: Approximation1(); break;
       case 4: Approximation2(); break;
       case 5: DP(); break;
       case 6: DP_general_rlcs_run(); break;

       default: std::cout << "Not the right algorithm number! Choose the right one from the set {0, 1, ... , 6} "; 
    }
    return;
}

void Algorithm:: Greedy(double t_lim, int  beta)
{
     this->BS(t_lim, 1);  // beta=1 pre-defined
}

void Algorithm:: BS(double t_lim, int beta)
{
   
    // root node initialization -- time limit not incorporated??
    std::vector<int> pL(this->inst->m, 0);
    std::vector<int> ppl(this->inst->p, 0);
    std::vector<int> rpl(this->inst->r, 0);
    //Node* nptr = nullptr;
    Node *root = new Node(inst, pL, ppl, rpl); // return;
    // create a beam, initialited with the root node
    std::vector<Node*> beam;
    beam.push_back(root);
    int beam_size = beta; // should be parametrized
    int level = 0;
    int l_best = 0;
    double time_monitoring = 0.0;
    Node* best_node = root;
    // instantitate the clock 
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::set<std::tuple<std::vector<int>, std::vector<int>>> node_in_set;

    while(!beam.empty())
    {  
         std::vector<Node*> V_ext;
         
         for(const auto & node: beam)
         {  
              std::vector<Node*> children_nodes = node->expansion(); 
              if(children_nodes.empty())
              {
                  if(node->l_v > l_best && node->is_complete()) // update the best solution (node has no children)
                  {  
                      l_best = node->l_v;
                      best_node = node;
                  }
              }
              // otherwise add those nodes in V_ext set (remove duplicated ones)
              for(auto& child : children_nodes) //add into V_ext (if not already in the set)
              {
                  if (node_in_set.find(std::make_tuple(std::get<0>(child->position), std::get<2>(child->position))) == node_in_set.end()) 
                  {    
                      V_ext.push_back(child);      
                      node_in_set.insert(std::make_tuple(std::get<0>(child->position), std::get<2>(child->position)));
                  } 
                  else 
                       continue;
              }
         }
         //sort V_ext 
         std::sort(V_ext.begin(), V_ext.end(), 
                  [](Node* const &n1, Node* const &n2){  
                     if(h_heuristic <= 2 || h_heuristic==4) //0-2: UB bounds applied or 4: Prob-based guidance
                     {          
                                  
                         if(n1->heuristic_value == n2->heuristic_value)
                         //    if(h_heuristic!=4)
                             return n1->secondary_value > n2->secondary_value;
                         //    else
                         //       return false;  
                         else
                            return n1->heuristic_value > n2->heuristic_value;    //comparison   
                     }
                     else
                         return   n1->heuristic_value < n2->heuristic_value;            
                  } 
         );
            
         // calculate k: if -h == 4: Prob based take only 33% of the best 
         if(h_heuristic==4)
         {
            int k=10000;  int nodes_counter=0;  
            for(auto& v : V_ext)
            {   
                int index=0;
                std::vector<int> pos = std::get<0>(v->position);
                
                for(int& pli: pos)
                {
                    if(k > (int)this->inst->S[index].size() - pli + 1)
                        k=this->inst->S[index].size() - pli + 1;
                 
                     index++; 
                }  
                nodes_counter++;
                if(nodes_counter > std::ceil(V_ext.size() * pct_v_ext)) // take only 33% from all V_ext (ordered by UB)
                    break;
            }

            if( double(k) / this->inst->Sigma < 1.0 )
                k=1;  
            else
                k=std::ceil( double(k) / this->inst->Sigma);
         
            for(auto& v: V_ext) // determine a Prob valued of a node
                v->set_up_prob_value(k);

                     //sort expanded  
            std::sort(V_ext.begin(), V_ext.end(), 
                  [](Node* const &n1, Node* const &n2){  
                      if(n1->heuristic_value == n2->heuristic_value)
                             return n1->secondary_value > n2->secondary_value; //R_min
                         else 
                            return n1->heuristic_value > n2->heuristic_value;    //comparison   
                  } 
            );
         }
         // the last phase: clean the old beam, replace it with a new best beta nodes fron V_ext
         beam.clear();
         int num_nodes_in_beam  = 0;
         for(auto const &v : V_ext)
         {  
             beam.push_back(v);
             num_nodes_in_beam++;
             if(num_nodes_in_beam >= beam_size) 
                 break;
         }
         level++;//go to the next level
         node_in_set.clear();
         // time limit monitoring:
         auto end_time = std::chrono::high_resolution_clock::now();
         // Calculate the duration in microseconds
         auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
         time_monitoring = double(duration.count()) / 1000;
         //std::cout << time_monitoring << std::endl;
         if(t_lim < time_monitoring)
             break;
    }
    this->running_time = time_monitoring;
    this->solution = best_node->export_solution();
    // write in an out-file: statistics extraction:
    this->save_in_file(outpathname);

}

void Algorithm:: A_star(double t_lim) 
{  

    std::cout << " RUN A* algorithm "  << std::endl;
    // Initialization
    std::unordered_map< std::tuple<std::vector<int>, std::vector<int>, std::vector<int> >, int, Hash> N;      // a grah structore to extract the solution
    std::priority_queue<Node*, std::vector<Node*>, NodeComparator> Q;    // open list -- not yet expanded nodes and list of all nodes so-far encoutered in the graph 
    // root node initialization        
    std::vector<int> pL(this->inst->m, 0);
    std::vector<int> ppl(this->inst->p, 0);
    std::vector<int> rpl(this->inst->r, 0);
    //Node* nptr = nullptr;
    Node *root = new Node(inst, pL, ppl, rpl, nullptr);
    double time_monitoring = 0.0;
    // instantitate the clock 
    auto start_time = std::chrono::high_resolution_clock::now();
    //initialize the lists with the root node information
    N[std::make_tuple(pL, ppl, rpl)] = 0;
    Q.push(root); // prioritized acc. to f_value

    try{

            while(!Q.empty() && time_monitoring < t_lim)
            {
                //run A* iteration:
                Node *p = Q.top();
                Q.pop();
                if(p->f_value <= (int)this->solution.size()) // no need to run the algorithm furhter (optimality of the best solution has been proven)
                    break;
                //otherwise expand the node
                std::vector<Node*> children = p->expansion();
        
                if(children.empty() && (int)this->solution.size() < p->l_v) // && p->is_complete() it can be complete but UB>0 
                {
                        this->solution.clear();
                        this->solution = p->export_solution();
                        this->running_time = time_monitoring;

                        /*p->print();
                        std::cout << "Parent..." << std::endl;
                        p->parent->print();*/
                        //break;
                        //return;  
                        continue;
                }

                // if a new primal bound has reached by A*:
                if((int) this->solution.size() < children[0]->l_v){
                    this->solution.clear();
                    this->solution = children[0]->export_solution(); // export the best primal solution so-far found
                    // update statistics:
                    this->time_best_solution_found = time_monitoring;
                    this->num_N_nodes = N.size();
                    this->num_Q_nodes = Q.size();
                    
                    this->save_in_file(outpathname);  // save every time new best result found

                }
                //add children nodes in respective lists: 
                // if node not in N ==> add it freely in N and Q

                for(auto& child: children)
                {
                    auto it = N.find(child->position);

                    if (it == N.end()) {
                        N[  child->position ] = child->l_v;
                        Q.push(child);
                    } else { // compare the maximum paths to that node ==> if better, update it
            
                        int best_sofar_lenght_to_node =  N[ child->position ];
                        if(best_sofar_lenght_to_node < child->l_v)
                        {
                            N[ child->position ] = child->l_v;
                            Q.push(child);
                        }
                        else //outdated node, just skip it
                            continue;
                     }
                }
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                time_monitoring = double(duration.count()) / 1000;
                this->running_time = time_monitoring; // update execution time

            }
            
            this->num_N_nodes = N.size();
            this->num_Q_nodes = Q.size();
            std::cout << "Save in the file " << std::endl;
            this->save_in_file(outpathname);

       } catch (const std::bad_alloc& e) {
        // Handle bad_alloc exception
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        //std::cout << "Save in the file " << std::endl;
        this->save_in_file(outpathname);  

        throw; // Rethrow the exception
    } catch (const std::exception& e) {
        // Catch any other exceptions
        std::cerr << "An error occurred: " << e.what() << std::endl;
        this->save_in_file(outpathname);
        throw; // Rethrow the exception
    }

}

void Algorithm::Approximation1()
{
    // Imlemented from the work from Gotthilf et al. (2010) -- two approximation algorithms for the RLCS problem
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<int>> Occ;
    for(int i = 0; i < this->inst->m; ++i)
    { 
        std::vector<int> occ_i;
        for(int s=0; s < this->inst->Sigma; ++s)
            occ_i.push_back(this->inst->occurances_string_pos_char[s][i][0]);
   
        Occ.push_back(occ_i);
    }
    //Step 2:
    std::vector<int> Occ_min, Cons, Val;
    for(int s=0; s < this->inst->Sigma; ++s) // through letters 
    {
        int val_min = 1000000;
        for(int i = 0; i < this->inst->m; ++i)
            if(val_min > Occ[i][s])
               val_min = Occ[i][s];
        // fill in Cons
        int min_r = 1000000;
        for(int k=0; k < this->inst->r; ++k)
        {
            int counter_for_s = std::count(this->inst->R[k].begin(), this->inst->R[k].end(), s );
            if(counter_for_s  == (int)this->inst->R[k].size())
               if((int)this->inst->R[k].size() < min_r)
                  min_r = this->inst->R[k].size();
        }
        Occ_min.push_back(val_min);
        Cons.push_back(min_r);
        Val.push_back(std::min(val_min, min_r - 1)); // Val(s), the minimum between Occ(s) and Cons(s) − 1
    }
    //Step 3: Find s ∈ Σ with maximal V al(s) and return s^{Val(s)}.
    int max_val_s = 0; int s_res = -1;
    for(int s=0; s < this->inst->Sigma; ++s)
         if(max_val_s < Val[s])
         {
             max_val_s = Val[s];
             s_res = s;
         }
    // constriction of the (approx.) solution
    for(int i=0; i< max_val_s; ++i)
        this->solution.push_back(s_res);

     // time measurement
     auto end_time = std::chrono::high_resolution_clock::now();
     auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
     double time_monitoring = double(duration.count()) / 1000;
     this->running_time = time_monitoring;
     // save info in the internal file 
     this->save_in_file(outpathname);
}

void Algorithm::initialize(auto& LCS, std::vector<int16_t>& pos)
{
    //std::cout << LCS.size() << std::endl;

    if((int)pos.size() == this->inst->m)
    { 
         // Copying vector by copy function  
         std::vector<int16_t> vect2;  
         std::copy(pos.begin(), pos.end(), std::back_inserter(vect2));  
         auto pair = std::make_pair(pos, 0);
         LCS[pos] = 0;       
         //std::cout << "|LCS|=" << LCS.size() << std::endl;  
         return;
    }
    for(int i = 0; i <= (int)this->inst->S[pos.size()].size(); ++i)
    {    
            pos.push_back((int16_t) i);  
            initialize(LCS, pos);
            // Uklanjamo poslednju dodatu cifru kako bismo isprobali sledeću
            pos.pop_back();
    }
}

 void Algorithm::DP_LCS(auto& LCS, std::vector<int16_t>& pos)
 {
     
      if((int)pos.size() == (int)this->inst->m)//check the two possible cases
      {
          
            std::vector<int16_t> pos_minus_1_all;
            std::copy(pos.begin(), pos.end(), std::back_inserter(pos_minus_1_all));
            for(int i = 0; i < this->inst->m; ++i)
                pos_minus_1_all[i] -= 1;
              
           bool match = true;
           for(int i=0; i < (this->inst->m - 1)&&match;++i)
           {
                if(this->inst->S[i][pos_minus_1_all[i] ] != this->inst->S[i+1][ pos_minus_1_all[i+1] ])
                    match =  false;
           }
           if(match)  //it is a match 
              LCS[pos] = 1 + LCS[pos_minus_1_all];
           else
           {
                // calculate LCS[pos] = max{LCS[pos_1-1, pos_2, ..., pos_m], LCS[pos1, pos2-1, ... , pos_m] , ...   }
                int max_val = 0;
                std::vector<int16_t> pos_minus_1;
                std::copy(pos.begin(), pos.end(), std::back_inserter(pos_minus_1));
                for(int i = 0; i < this->inst->m; ++i)
                {
                     pos_minus_1[i] -= 1;
               
                     if(LCS[pos_minus_1] >  max_val)
                         max_val = LCS[pos_minus_1];
               
                     pos_minus_1[i] += 1; // revert the value back
                }
                //std::cout << "max_val " << max_val << " " << pos[0] <<" " << pos[1] << " " << pos[2] << std::endl;
                LCS[pos] = max_val;
           }
           return;
      }
      for(int index = 1; index <= (int)this->inst->S[pos.size()].size(); ++index)
      {
            pos.push_back((int16_t)index);
            DP_LCS(LCS, pos);
            pos.pop_back();
      }
 }

void Algorithm::extract_lcs_solution(auto& LCS, std::vector<int16_t>& pos)
{
    
    if(std::count(pos.begin(), pos.end(), 0) > 0)
        return;

    bool match = true;
    for(int i=0; i < (this->inst->m - 1)&&match;++i)
    {
        if(this->inst->S[i][pos[i] - 1] != this->inst->S[i+1][ pos[ i+1 ] -1 ])
           match =  false;
    }
    if(match)  //it is a match 
    {
        this->solution.insert(this->solution.begin(), this->inst->S[0][pos[0] - 1]);
        for(int i=0; i < this->inst->m; ++i)
            pos[i]--;
        
        extract_lcs_solution(LCS, pos);
    }
    else{ // move to one of the sub-problems --
        
        std::vector<int16_t> pos_minus_1;
        std::copy(pos.begin(), pos.end(), std::back_inserter(pos_minus_1));
        for(int i = 0; i < this->inst->m; ++i)
        {
            pos_minus_1[i] -= 1;
               
            if(LCS[ pos ] == LCS[ pos_minus_1 ])
            {
                extract_lcs_solution(LCS, pos_minus_1);
                break;
            }                 
            pos_minus_1[i] += 1; // revert the value back
        }
    }  
}

void Algorithm::Approximation2()
{
    //Approximation algorithm 2:
    // Step 1: Compute LCS  S for A1, ... ,, A_m 
    // Step 2: return the prefix S[0:k_{min}-1] as the final (approx) solution

    // Step 1: realization  LCS recursion -- initialize
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<int16_t> vx;    std::unordered_map<std::vector<int16_t>, int16_t, VectorHasher> LCS;
    this->initialize(LCS, vx); 
    // recursion for LCS 
    vx.clear(); 
    //Step 2: recursion
    this->DP_LCS(LCS,  vx);    
    std::vector<int16_t> vec_final;
    for(int i=0; i < this->inst->m; ++i)
        vec_final.push_back((int16_t)this->inst->S[i].size());

    //Stats:
    std::cout << "\nObjective: " << LCS[vec_final] << std::endl;
    std::cout << "Solution: ";
    //back-track solution: extraction
    this->extract_lcs_solution(LCS, vec_final);
    std::cout << this->solution.size() << std::endl;
    this->additional_info = this->solution.size();

    //Step 2: final -- cut off the solution to the leading k_{min}-1 chars
    int k_min = 100000;
    for(int k=0; k<this->inst->r; ++k)
        if(k_min > (int)this->inst->R[k].size())
           k_min = this->inst->R[k].size();

    std::cout << "k_min-1: " << (k_min - 1) << std::endl;
    std::vector<int> prefix;
    std::copy(this->solution.begin(), this->solution.begin() + (k_min - 1), std::back_inserter(prefix));
    // copy prefix to this->solution 
    this->solution.clear();
    std::copy(prefix.begin(), prefix.begin() + (k_min - 1), std::back_inserter(this->solution));

    for(auto& c: this->solution)
        std::cout << c << " ";
    std::cout << "\nFeasible " << validate_solution() << std::endl;
     // time measurement
     auto end_time = std::chrono::high_resolution_clock::now();
     auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
     double time_monitoring = double(duration.count()) / 1000;
     this->running_time = time_monitoring;
     // save the info in the internal file 
     this->save_in_file(outpathname);
}

void Algorithm::extract_DP_RLCS_solution(auto& RLCS, std::vector<int>& pos)
{
    //TODO
    int i = pos[0];
    int j = pos[1];
    int k = pos[2];

    if(i== 0 || j==0)// a trivial case    
        return;
    if(k==0)//TODO: do like for LCS
    { 
        //std::vector<int> pos_left; pos_left.push_back(j); pos_left.push_back(k); 
        //std::cout << i <<" " << j <<  " " << RLCS[pos_left] << std::endl;

        if(this->inst->S[0][i-1] == this->inst->S[1][j-1] ) //match
        {
            this->solution.push_back(this->inst->S[0][i-1]);
            std::vector<int> pos_ij; pos_ij.push_back(i-1);pos_ij.push_back(j-1); pos_ij.push_back(0);
            extract_DP_RLCS_solution(RLCS, pos_ij);
        }else{

            std::vector<int> pos1; pos1.push_back(i-1);pos1.push_back(j); pos1.push_back(0);
            std::vector<int> pos2; pos2.push_back(i); pos2.push_back(j-1); pos2.push_back(0);
            
            if(RLCS[pos] == RLCS[pos1])
            {
                extract_DP_RLCS_solution(RLCS, pos1);
            }else{
                extract_DP_RLCS_solution(RLCS, pos2);
            }
        }

       return;
    }
    // the first recurrence branch case:
     std::vector<int> pos_left;  pos_left.push_back(i); pos_left.push_back(j); pos_left.push_back(k);

    if(this->inst->S[0][i-1] == this->inst->S[1][j-1] && (this->inst->S[0][i-1]  == this->inst->R[0][k-1]) ) //two subproblems to consider
    {
        std::vector<int> pos_mid;  pos_mid.push_back(i-1); pos_mid.push_back(j-1); pos_mid.push_back(k);
        std::vector<int> pos_right;  pos_right.push_back(i-1); pos_right.push_back(j-1); pos_right.push_back(k-1);
        
        if(RLCS[pos_left] == RLCS[pos_mid])
        {
            extract_DP_RLCS_solution(RLCS, pos_mid);
            return;
        }
        else 
        if(RLCS[pos_left] == (1 + RLCS[pos_right]))
        {
            this->solution.push_back(this->inst->S[0][i-1]);
            extract_DP_RLCS_solution(RLCS, pos_right);
            return;
        }
    }else 
     if(this->inst->S[0][i-1] == this->inst->S[1][j-1] && (this->inst->S[0][i-1]  != this->inst->R[0][k-1]) ) //two subproblems to consider
     { // case 2 of the recursion's branch
         std::vector<int> pos_right;  pos_right.push_back(i-1); pos_right.push_back(j-1); pos_right.push_back(k);
         this->solution.push_back(this->inst->S[0][i-1]);
         extract_DP_RLCS_solution(RLCS, pos_right);
         return;

     }else{ // ai != bj

         std::vector<int> pos_mid;  pos_mid.push_back(i-1); pos_mid.push_back(j); pos_mid.push_back(k);
         std::vector<int> pos_right;  pos_right.push_back(i); pos_right.push_back(j-1); pos_right.push_back(k);

         if(RLCS[pos_left] == RLCS[pos_mid])
         {  extract_DP_RLCS_solution(RLCS, pos_mid);   return; }
         else if(RLCS[pos_left] == RLCS[pos_right])
         {  extract_DP_RLCS_solution(RLCS, pos_right);   return; }     
     }
}
void Algorithm::DP()
{

    // The paper of Deorowitcz, 2014: the problem of the longest common subsequence of two sequences in which a constraint
    // is forbidden to be found as a subsequence of the result
    if(this->inst->m > 2)
    {
    std::cerr << "\nError inappropriate instance size: this aproach is only for the case of m=2 and k=1" << std::endl;
       return;
    }
    
    auto start_time = std::chrono::high_resolution_clock::now();
    std::unordered_map<std::vector<int>, int, VectorHasher> RLCS;
    //initialize 
    for(int i=0; i <= (int)this->inst->S[0].size(); ++i)
    {
        for(int k=1; k <= (int)this->inst->R[0].size(); ++k)
        {
            std::vector<int> pos;
            pos.push_back(i); pos.push_back(0); pos.push_back(k);
            RLCS.insert({{pos, 0}});
        }
    }
    
    for(int j=0; j <= (int)this->inst->S[1].size(); ++j)
    {
        for(int k=1; k <= (int)this->inst->R[0].size(); ++k)
        {
            std::vector<int> pos;
            pos.push_back(0); pos.push_back(j); pos.push_back(k);
            RLCS.insert({{pos, 0}});
        }
    }

    for(int i=0; i <= (int)this->inst->S[0].size(); ++i)
    {
        for(int j=0; j <= (int)this->inst->S[1].size(); ++j)
        {
            std::vector<int> pos;
            pos.push_back(i); pos.push_back(j); pos.push_back(0);
            RLCS.insert({{pos, -1000000}});
        }
    }
    //run a bottom-up strategy -- tabularization for DP:
    for(int i=1; i <= (int)this->inst->S[0].size(); ++i)
    {
        for(int j=1; j <= (int)this->inst->S[1].size(); ++j)
        {
            for(int k=1; k <= (int)this->inst->R[0].size(); ++k)
            {
                 std::vector<int> pos_left;  pos_left.push_back(i); pos_left.push_back(j); pos_left.push_back(k);
                 if(this->inst->S[0][i-1] == this->inst->S[1][j-1] && (  this->inst->S[0][i-1]  == this->inst->R[0][k-1]  ) )
                 {
                        std::vector<int> pos_mid;  pos_mid.push_back(i-1); pos_mid.push_back(j-1); pos_mid.push_back(k);
                        std::vector<int> pos_right;  pos_right.push_back(i-1); pos_right.push_back(j-1); pos_right.push_back(k-1);
                        RLCS[pos_left] = std::max(RLCS[pos_mid], 1 + RLCS[pos_right]);
                 }
                 else if(this->inst->S[0][i-1] == this->inst->S[1][j-1] && ( this->inst->S[0][i-1]  != this->inst->R[0][k-1]   ) )
                 {
                        std::vector<int> pos_right;  pos_right.push_back(i-1); pos_right.push_back(j-1); pos_right.push_back(k);
                        RLCS[pos_left] = 1 + RLCS[pos_right];
                        
                 }else{
                        std::vector<int> pos_mid;  pos_mid.push_back(i-1); pos_mid.push_back(j); pos_mid.push_back(k);
                        std::vector<int> pos_right;  pos_right.push_back(i); pos_right.push_back(j-1); pos_right.push_back(k);         
                        RLCS[pos_left] = std::max(RLCS[pos_mid], RLCS[pos_right]);
                 }
            }
        }
    }
    std::vector<int> final_pos;
    final_pos.push_back(this->inst->S[0].size());    final_pos.push_back(this->inst->S[1].size()); final_pos.push_back(this->inst->R[0].size());
    std::cout << "\nObjective: " << RLCS[final_pos] << std::endl;
    // extract the (optimal) solution
    this->extract_DP_RLCS_solution(RLCS, final_pos);
    std::cout << "Solution: " << std::endl;//<< this->solution.size() << std::endl;
    std::reverse(this->solution.begin(), this->solution.end());
    for(auto c: this->solution)
        std::cout << c << " ";
    std::cout << "" << std::endl;

    // save in the file
    auto end_time = std::chrono::high_resolution_clock::now();
    // Calculate the duration in microseconds
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    double time_monitoring = double(duration.count()) / 1000;

    this->running_time = time_monitoring;
    this->save_in_file(outpathname);
}

// DP general for RLCS problem 
/*void Algorithm::initialize_general_rlcs(auto& RLCS, std::vector<int>& starting_vector)
{
     std::cout <<  RLCS.size() << std::endl;
     if((int)starting_vector.size() == (int)this->inst->m + this->inst->r)
     {
         RLCS[starting_vector] = 0;
         return;
     }
     if((int)starting_vector.size() < (int)this->inst->m) // S[starting_vector.size()] -- vector
     {
         for(int ix=0; ix <= (int)this->inst->S[(int)starting_vector.size()].size(); ++ix)
         {
             starting_vector.push_back(ix);
             initialize_general_rlcs(RLCS, starting_vector);
             starting_vector.pop_back();
         }
     }else{ // R[starting_vector.size()) - this->inst->m]
          
          for(int rx=0; rx <= (int)this->inst->R[starting_vector.size()-this->inst->m].size(); ++rx)
          {
              starting_vector.push_back(rx);
              initialize_general_rlcs(RLCS, starting_vector);
              starting_vector.pop_back();
          }
     }
}*/

void Algorithm::extract_solution_general_rlcs(auto& RLCS, std::vector<int16_t>& final_state)
{
    
    //trivial cases 
    bool zeros_in_s = false;
    for(std::vector<int16_t>::iterator it = final_state.begin(); it != (final_state.begin() + this->inst->m); ++it)
        if(*it == 0){
            zeros_in_s = true;
            break;
        }
    if(zeros_in_s) // break the recursion
        return;

    int k_ones =0;
    for(std::vector<int16_t>::iterator it = (final_state.begin() + this->inst->m); it != final_state.end(); ++it)
        if(*it == 1)
           k_ones++;

    // recursion:
    bool match = true;
    for(int i=0; i < (this->inst->m-1)&&match; ++i)
         if(this->inst->S[i][ final_state[i] - 1 ] != this->inst->S[i+1][ final_state[i+1] - 1 ])
            match=false;
    //match or not??
    if(!match)
    {
        std::vector<int16_t> subproblem;
        std::copy( final_state.begin(), final_state.end(), std::back_inserter(subproblem));

        for(int i=0; i < this->inst->m; ++i)
        {
            subproblem[i]-=1;
            if(RLCS[subproblem] == RLCS[final_state])
            {
                extract_solution_general_rlcs(RLCS, subproblem);
                break;
            }
             subproblem[i]+=1;
        }
    }else{ // check if a MATCH 
 
        int letter = this->inst->S[0][final_state[0] - 1 ];
        bool match_with_r = true;
        for(int k=0; k < this->inst->r&&match_with_r; ++k)
            if(this->inst->R[k][ final_state[ this->inst->m + k ] - 1 ] != letter )
                match_with_r = false;
        
        std::vector<int16_t> subproblem;
        std::copy( final_state.begin(), final_state.end(), std::back_inserter(subproblem));
        if(!match_with_r) //not a full match but only between S-strings
        {
            for(int i=0; i < this->inst->m; ++i)
                subproblem[i]-=1;
            // update positions of R_k-strings
            if(RLCS[subproblem] == RLCS[final_state])
               extract_solution_general_rlcs(RLCS, subproblem);
            else{
                for(int rx=0; rx < this->inst->r; ++rx)
                    if(this->inst->R[rx][ final_state[this->inst->m + rx ] - 1]  == letter )
                       subproblem[this->inst->m + rx] -=1;
            
                this->solution.push_back(letter);
                extract_solution_general_rlcs(RLCS, subproblem);
            }
            
        } else{ // FULL match with R strings
             
             int letter = this->inst->S[0][final_state[0] - 1 ];
             for(int i=0; i < this->inst->m; ++i)
                 subproblem[i] -=1;
             for(int rx=0; rx < this->inst->r; ++rx)
                 subproblem[this->inst->m + rx] -= 1;
  
             if(1 + RLCS[subproblem] == RLCS[final_state])
             {
                     this->solution.push_back(letter);
                     extract_solution_general_rlcs(RLCS, subproblem);

             }else{
                  for(int rx=0; rx < this->inst->r; ++rx)
                      subproblem[this->inst->m + rx] =  final_state[this->inst->m + rx ];
                   extract_solution_general_rlcs(RLCS, subproblem);
                }             
        }
    }
}

void Algorithm::DP_general_rlcs_run()
{
     
     auto start_time = std::chrono::high_resolution_clock::now();
     std::unordered_map<std::vector<int16_t>, int16_t, VectorHasher> RLCS;
     //std::vector<int> starting_vector;
     //this->initialize_general_rlcs(RLCS, starting_vector);
     //main recursion -- tabularization 
     
     std::vector<int16_t> final_vector;
     for(int i=0; i < this->inst->m; ++i)
         final_vector.push_back(this->inst->S[i].size());
     for(int rx=0; rx < this->inst->r; ++rx)
         final_vector.push_back(this->inst->R[rx].size());
     
     std::cout << "\nRun DP for RLCS ...\n Final position vector: " << std::endl;
     for(auto x: final_vector)
         std::cout << x <<  " " << std::endl;  
     
     int optimum = DP_general_recursion(RLCS, final_vector); 

     std::cout << "Optimum: " << optimum << std::endl;
    // TODO: extract the optimal solution ??
    this->extract_solution_general_rlcs(RLCS, final_vector);
    // reverse the solution:
    std::reverse(this->solution.begin(), this->solution.end());
    auto end_time = std::chrono::high_resolution_clock::now();
    // Calculate the duration in microseconds
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    double time_monitoring = double(duration.count()) / 1000;
    std::cout << "Time: " << time_monitoring << std::endl;
    // save it in the out-file
    this->running_time = time_monitoring;
    this->save_in_file(outpathname);

}  
int Algorithm::DP_general_recursion(auto& RLCS, std::vector<int16_t>& final_vector)
{
     
     if(RLCS.find(final_vector)!= RLCS.end()) // already determined
        return RLCS[final_vector];

     // trivial cases
     int count_zeros=0; 
     for(std::vector<int16_t>::iterator it = final_vector.begin(); it != final_vector.begin() + this->inst->m; ++it)//basic case 
        if(*it == 0)   
            count_zeros++;
     
     if(count_zeros > 0) // the first basic case 
     {
          RLCS.insert({{final_vector, 0}});
          return 0; // break the recursion 
     }

     bool match = true;
     for(int i=0; i < (this->inst->m-1)&&match; ++i) 
         if(this->inst->S[i][ final_vector[i] - 1 ] != this->inst->S[i+1][ final_vector[i+1] - 1 ])
            match=false;
     
    std::vector<int16_t> subproblem; int16_t max_value_for_original_problem = -1; //copy ==> needs to bel cleared up afterwards 
    std::copy(final_vector.begin(), final_vector.end(), std::back_inserter(subproblem));
    
    if(!match) //not a match
    {  
        for(int i=0; i < this->inst->m; ++i)
        {
             subproblem[i] -= 1;
             int val_subproblem = DP_general_recursion(RLCS,  subproblem);
             if(max_value_for_original_problem < val_subproblem)
                max_value_for_original_problem = val_subproblem;
             subproblem[i] += 1;
        }
        subproblem.clear();
        RLCS.insert({ {final_vector, max_value_for_original_problem} });  
        return RLCS[final_vector] ; 
    }     
    else{ //if a match has been encoutered ... act like in the case of Lemma 1 (by Gotthilf et al., 2010)

        int letter = this->inst->S[0][final_vector[0]-1]; 
        bool k_one_exist=false;

        for(int rx=0; rx < this->inst->r &&!k_one_exist; ++rx)
        { 
            if(final_vector[this->inst->m + rx ] == 1 &&  this->inst->R[rx][ final_vector[this->inst->m + rx ] - 1]  == letter) //will derease to 0
                k_one_exist=true;
        }

        for(int i=0; i < this->inst->m; ++i)
             subproblem[i] -= 1;
        // update positions of R_k-strings
        int16_t max_sub_r_string1 =  DP_general_recursion(RLCS, subproblem);
        // the second part -- applies in case k_one_exist == false
        int16_t max_sub_r_string2  = 0; 
        if(k_one_exist)
           max_sub_r_string2 =  0;// DP_general_recursion(RLCS, subproblem);
        else // j_x* are the real values ==> add the matching character (part of the optimal solution)
        {
                
            for(int rx=0; rx < this->inst->r; ++rx ) //&& !decreasing_to_zero_happened; ++rx)  
                if(subproblem[this->inst->m + rx ] - 1 >0 && this->inst->R[rx][ subproblem[this->inst->m + rx ] - 1]  == letter )
                   subproblem[this->inst->m + rx] -= 1;
                
            max_sub_r_string2 = 1 + DP_general_recursion(RLCS, subproblem); 

        }
             
        subproblem.clear();
        RLCS[final_vector] =  std::max(max_sub_r_string1, max_sub_r_string2);   

        return RLCS[final_vector] ; 
     }  
}
bool Algorithm:: validate_solution()
{

    if(this->solution.size() == 0)
        return false;

    for(int i=0; i < this->inst->m; ++i)
    {
         int count = 0;
         for(auto& si: this->inst->S[i])
         {
             if(si == this->solution[count])
                 count++;
             if(count == (int)this->solution.size())
                break; // move to the next string si
         }
         if(count < (int)this->solution.size())
         {
             std::cout << "string " << i << " is not a supersequence of the solution "
                       <<  count << " and needs to be " << this->solution.size() <<  std::endl;
             return false;
         }
    }
    // P-string incluion
    for(int j=0; j < this->inst->p; ++j)
    {
        int count = 0;
        for(auto& s: this->solution)
        {
            if(this->inst->P[j][count] == s)
                count++;
            if(count == (int)this->inst->P[j].size())
               break;
        }
        if(count < (int)this->inst->P[j].size())
            return false;
    }
    // R-strings -- non-inclusion
    for(int k=0; k < this->inst->r; ++k)
    {
        int count = 0;
        for(auto& s: this->solution)
        {
            if(s == this->inst->R[k][count])
                count++;
            if(count == (int)this->inst->R[k].size())
                return false;
        }
   }    
   return true;
}


void Algorithm::save_in_file(const std::string& outfile)
{
    //clean filename
    std::string clean_file_name = "";
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
        std::cout << "Feasible: " << this->validate_solution() << std::endl;
        return;
    }
     std::ofstream outputFile(outfile);
     // Check if the file is opened successfully
     if (!outputFile.is_open()) {
          std::cerr << "Error opening the file." << std::endl;
          return; // Return an error code
     }
     // write in an out-file
     outputFile << out_terminal_string << std::endl;
     outputFile << "Objective: " << this->solution.size() << std::endl;
     outputFile << "Solution: ";
     for(auto& c: this->solution)
         outputFile << this->inst->int2char[c] << " " ;

     outputFile << "\nTime: " << this->running_time << std::endl;
     outputFile << "Time best found: " << this->time_best_solution_found << std::endl;
     outputFile << "Feasible: " << this->validate_solution() << std::endl;
     if(this->number_algorithm == 2) // it is the A* search
        outputFile << "Number of nodes created: " << this->num_N_nodes << "\t number of Q nodes: " << this->num_Q_nodes << std::endl;
     // Close the file
     if(this->number_algorithm ==4) //APPROX2
        outputFile << "LCS size between S-strings: " << this->additional_info  << std::endl;
     outputFile.close();
}
