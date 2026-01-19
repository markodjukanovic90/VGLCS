#pragma once
#include<vector>
#include<map>
#include "functions.h"

using rlcs_position = std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>;  

class Node
{
   
   public:
           
        Instance* inst;
        rlcs_position position; // all positional vectors stored
        double heuristic_value; // primary criterion
        double secondary_heuristic_value; //prob. value in case of ties with the neural network 
        int32_t f_value; // used of A* search
        Node* parent; // keep store a parent node of this node
        int l_v;
        bool complete = false;
        std::vector<double> features; //features for the node

   public:
   
        Node(Instance *instance, std::vector<int>& left, std::vector<int>& ppos, std::vector<int>& rpos, Node* parent=nullptr);
        // helper functions
        double greedy_function(); // calculates a greedy value -- used of Greedy algorithm
        void set_up_prob_value(int k);
        bool domination_two_letters(rlcs_position& posA, rlcs_position& posB);
        std::pair<std::map<int, rlcs_position>, std::map<int, rlcs_position>> sigma_feasible_letters(); // TODO
        std::vector<Node*> expansion();
        std::vector<int> export_solution(); // export the best solution that refers to this node
        bool is_complete();
        // os noda a complete one or not
        void print(); // print the basic info for the node
         // Overload less-than operator for priority_queue
        bool operator> (const Node* other);
        //~Node();
    
};