#include "Node.h"
#include "globals.h"
#include<iostream>
#include <tuple>
//#include <bits/stdc++.h>


/**
 * DECLARING ENUM TYPES FOR HEURISTICS 
*/
enum Heuristics { UB_1, UB_2, UB_min, G,  Prob }; 


double Node::greedy_function()
{
      //if(std::get<0>(this->position)[0] == 0)
      if(this->parent == nullptr)
         return 0.0;
      
      double greedy = 0.0;
      for(int i=0; i < this->inst->m;++i)
      {
           int prev_char_pos_i = std::get<0>(this->parent->position)[i];
           int current_char_pos_i =  std::get<0>(this->position)[i];
           int skiped_for_i = current_char_pos_i - prev_char_pos_i - 1;
           int remaining = (int)this->inst->S[i].size() - std::get<0>(this->position)[i] + 1;
           double score_i = ((double) skiped_for_i) /  remaining;
           
           greedy += score_i;
      }
       // include  P-strings 
      for(int j = 0; j < this->inst->p; ++j)
      {
           int curr_pos_p_j = std::get<1>(this->position)[j];
           int prev_pos_p_j = std::get<1>(this->parent->position)[j];

           if( curr_pos_p_j - prev_pos_p_j == 1 || curr_pos_p_j  == (int)this->inst->P[j].size() )// matched -- not penalized
               continue;
           else
               greedy += 1.0/( (int)this->inst->P[j].size() - curr_pos_p_j ) * this->inst->Sigma;
      }
      
       return greedy;
}
 

Node::Node(Instance *instance, std::vector<int>& left, std::vector<int>& ppos, std::vector<int>& rpos, Node* parent)
{
    
    // fill in the instance attributes
    this->inst = instance;
    this->position = std::make_tuple(left, ppos, rpos); // set up positions
    this->parent = parent;
    //set up the l_v-values   
    if(parent == nullptr)
       this->l_v = 0;
    else
    {
       this->l_v = this->parent->l_v + 1;
    }
    // calculate f-value of the subproblem
    this->f_value = 0;
    this->f_value = this->l_v + this->heuristic_value;

}

void Node::set_up_prob_value(int k)
{
     
    HeuristicFunction *hf  = new Probability(k);     //std::cout <<  "k---- " << k << std::endl;

    this->secondary_heuristic_value = binary_op(std::get<0>(this->position), std::get<1>(this->position), std::get<2>(this->position), 0, this->inst, hf);  

    delete hf;
}

/**  Returns true if the first letter is dominated by the second **/
bool Node::domination_two_letters(rlcs_position& posA, rlcs_position& posB)
{    
     // pL-vector
    for(int i=0; i<this->inst->m; ++i)
        if(std::get<0>(posA)[i] < std::get<0>(posB)[i])
            return false;
     
     // l_left
     for(int j = 0; j < this->inst->p;++j)
         if(std::get<1>(posA)[j] > std::get<1>(posB) [j] )
             return false;
     // r-left -- no need to checking  
     for(int i=0; i<this->inst->r; ++i)
        if(std::get<2>(posA)[i] <= std::get<2>(posB)[i])
            return false;

     return true;
}

std::pair<std::map<int, rlcs_position>, std::map<int, rlcs_position>> Node::sigma_feasible_letters()
{
    
    // addears in all remaining sufixes w.r.t. position pL
    std::vector<int> sigma;
    for(int l = 0; l < this->inst->Sigma;++l)
    {
         bool feasible = true;
         for(int i = 0; i < this->inst->m && feasible; ++i) // present in each remaining string S
         {

               if(std::get<0>( this->position)[i] >= (int)this->inst->S[i].size())
               {
                   feasible=false;
                   break;
               }
               int occur_l_in_si = this->inst->occurances_string_pos_char[l][i][ std::get<0>( this->position)[i] ];
               
               if( occur_l_in_si <= 0)
                   feasible = false;
         }
         
         if(feasible)
             sigma.push_back(l);
    }
    //std::cout <<"\sigma\=" << sigma.size() << std::endl;
    // filter acc- to remaining (suffices) of P-strings  l ==> pos
    std::map<int, rlcs_position> letters_next_positions;
    std::map<int, rlcs_position> letters_next_positions_skip;

    std::vector<int> letters_to_remove; std::vector<int> letters_to_remove_skip; // letters to be removed from the set of feas. extensions

    for(int lett: sigma)
    {
        
        std::vector<int> pl_next;  std::vector<int> pleft_next; std::vector<int> rleft_next; //std::vector<int> rleft_next_skip;

        for(int i = 0; i < this->inst->m; ++i)
        {
            int pl_i = this->inst->next_char_occurance_in_strings[lett][i][ std::get<0>(this->position)[i] ] + 1;
            pl_next.push_back(pl_i);
        }
        // update of the left positions of P-strings
        for(int j=0; j < (int)this->inst->P.size();++j)
        {
                 
                 int pj_left = std::get<1>(this->position)[j];
                 if(  (int)this->inst->P[j].size() > pj_left &&   this->inst->P[j][ pj_left ] == lett)
                    pleft_next.push_back(pj_left + 1);
                 else
                    pleft_next.push_back(pj_left);     
        }
        //update the left vector of R-strings
        //bool skip = false; 
        for(int k=0; k< (int)this->inst->R.size();++k)
        { 
                int rk_left = std::get<2>(this->position)[k];

                if(  this->inst->R[k][ rk_left ] == lett)
                {
                    rleft_next.push_back(rk_left + 1);  
                    //skip=true; 
                }   
                else
                    rleft_next.push_back(rk_left);  

                if(rleft_next[k] >= (int)this->inst->R[k].size())
                    letters_to_remove.push_back(lett); //letters_to_remove_skip??
        }

        letters_next_positions.emplace(lett, std::make_tuple(pl_next, pleft_next, rleft_next));
        //if(skip) // add 
        //   letters_next_positions_skip.emplace(lett,  std::make_tuple(pl_next, pleft_next, std::get<2>(this->position)));
    }
    //std::cout << "letters_next_positions lenght " << letters_next_positions.size() << std::endl;
    // Embed structure check ()  remaining_patern_suffix_pos[i,j, px] -- a max index of si for which Pj[px, |P|] can be imputed in si[i, |si|]

    for(std::map<int, rlcs_position>::iterator  it = letters_next_positions.begin(); it != letters_next_positions.end(); ++it)
    {    
         // std::find function call
         auto it_v = std::find(letters_to_remove.begin(),  letters_to_remove.end(), it->first);        

         if (it_v == letters_to_remove.end()) 
         {
            std::vector<int> pleft = std::get<1>(it->second);     
            bool feasible = true; 
            for (int j = 0; j < (int)pleft.size() && feasible; ++j)
            {  
                std::vector<int> pl_left = std::get<0>(it->second); //std::cout << "pleft ---" << pleft[0] << std::endl;
                for(int i = 0; i < this->inst->m && feasible; ++i)
                {
                    //std::cout << pleft[j] << " " << this->inst->remaining_patern_suffix_pos[ i ][ j ][ pleft[j] ] << " " <<  pl_left[i]<< std::endl;
                    if(pl_left[j] < (int)this->inst->P[j].size() && this->inst->remaining_patern_suffix_pos[ i ][ j ][ pleft[j] ] <  pl_left[i])   // i, j, pleft[j]
                       feasible = false;
                } 
            }
            if(!feasible)
                letters_to_remove.push_back(it->first);
         }
    }

    //REMOVE DOMINATION NODES FROM letter_next_postitions_skip
    /*for(std::map<int, rlcs_position>::iterator  it = letters_next_positions_skip.begin(); it != letters_next_positions_skip.end(); ++it)
    {    
         // std::find function call
         auto it_v = std::find(letters_to_remove.begin(),  letters_to_remove.end(), it->first);        

         if (it_v == letters_to_remove.end()) 
         {
            std::vector<int> pleft = std::get<1>(it->second);     
            bool feasible = true; 
            for (int j = 0; j < (int)pleft.size() && feasible; ++j)
            {  
                std::vector<int> pl_left = std::get<0>(it->second); //std::cout << "pleft ---" << pleft[0] << std::endl;
                for(int i = 0; i < this->inst->m && feasible; ++i)
                {
                    //std::cout << pleft[j] << " " << this->inst->remaining_patern_suffix_pos[ i ][ j ][ pleft[j] ] << " " <<  pl_left[i]<< std::endl;
                    if(pl_left[j] < (int)this->inst->P[j].size() && this->inst->remaining_patern_suffix_pos[ i ][ j ][ pleft[j] ] <  pl_left[i])   // i, j, pleft[j]
                       feasible = false;
                } 
            }
            if(!feasible)
                letters_to_remove_skip.push_back(it->first);
         }
    }*/

    // remove non-feasible letters from letters_next_positions:
    //std::cout << "letter_to_remove size " << letters_to_remove.size() << std::endl;
    for(auto letter_to_remove :  letters_to_remove)
         letters_next_positions.erase(letter_to_remove);

    //for(auto letter_to_remove :  letters_to_remove)
    //     letters_next_positions.erase(letter_to_remove);
    
    //  Filter DOMINATED nodes
    for(std::map<int, rlcs_position>::iterator  itA = letters_next_positions.begin(); 
                                                itA != letters_next_positions.end(); ++itA)
    {
        for(std::map<int, rlcs_position>::iterator  itB = letters_next_positions.begin(); 
                                                itB != letters_next_positions.end(); ++itB)
        {

            if(itA != itB) // check domination of two different leters if a dominates b, remove letter b
             {

                bool a_dominatedBy_b = this->domination_two_letters(itA->second, itB->second);
                if(a_dominatedBy_b)
                {   //std::cout << itA->first << " dominated by " <<  itB->first  << std::endl; 
                    letters_to_remove.push_back(itA->first); }
             }
        }
    }

    //  Filter DOMINATED nodes in letters_next_positions_skip (split as here the value of patial solution is l_v and above is l_v + 1 )
    /*for(std::map<int, rlcs_position>::iterator  itA = letters_next_positions_skip.begin(); 
                                                itA != letters_next_positions_skip.end(); ++itA)
    {
        for(std::map<int, rlcs_position>::iterator  itB = letters_next_positions_skip.begin(); 
                                                itB != letters_next_positions_skip.end(); ++itB)
        {
            if(itA != itB) // check domination of two different leters if a dominates b, remove letter b
             {

                bool a_dominatedBy_b = this->domination_two_letters(itA->second, itB->second);
                if(a_dominatedBy_b)
                {   //std::cout << itA->first << " dominated by " <<  itB->first  << std::endl; 
                    letters_to_remove_skip.push_back(itA->first); }
             }
        }
    }*/
    // filter letters_to_remove
    for(int leter_remove: letters_to_remove)
         letters_next_positions.erase(leter_remove);

    // remove from letters_next_positions_skip
    //for(int leter_remove: letters_to_remove_skip)
    //     letters_next_positions_skip.erase(leter_remove);
    
    //std::cout << "letters_next_positions " << letters_next_positions.size() << std::endl;
    // create (feasible) nodes 
    /*
    std::vector<Node*> node_extensions;
    for(std::map<int, rlcs_position>::iterator  it = letters_next_positions.begin(); it != letters_next_positions.begin(); ++it)
    {  
        int h_val = 0.0;
        Node* v_ext_a = new Node(this->inst, std::get<0>(it->second), std::get<1>(it->second), std::get<2>(it->second), h_val); //heap memory
        node_extensions.push_back(v_ext_a);
    }*/
    //std::cout <<"size of letters_next_positions " << letters_next_positions.size() << std::endl;
    return std::make_pair(letters_next_positions, letters_next_positions_skip);       
}

std::vector<Node*> Node:: expansion()
{
    //std::cout <<  "expad the node" << std::endl;
    auto positions = this->sigma_feasible_letters();
    std::map<int, rlcs_position> letters_next_positions = std::get<0>(positions);
    std::map<int, rlcs_position> letters_next_positions_skip = std::get<1>(positions);

    std::vector<Node*> node_extensions;        
    //int flag = 0; //  l_v = l_v + 1
    for(std::map<int, rlcs_position>::iterator  it = letters_next_positions.begin(); it != letters_next_positions.end(); ++it)
    {        
        Node* v_ext_a = new Node(this->inst, std::get<0>(it->second), std::get<1>(it->second), std::get<2>(it->second), this); //, flag); //heap memory
        node_extensions.push_back(v_ext_a);  
    }
    return node_extensions;
}

bool Node::is_complete() // assuming that it has no child nodes ...
{

    std::vector<int> pl = std::get<0>(this->position);
    std::vector<int> lleft = std::get<1>(this->position);
    std::vector<int> rleft = std::get<2>(this->position);
        
    int i = 0; // index iterator
    for( i=0; i < (int)lleft.size(); i++)
        if(lleft[i] < (int)this->inst->P[i].size())
            return false;

    int k = 0;
    for(auto& rleft_i: rleft)
    {
        if(rleft_i >= (int)this->inst->R[k].size())
            return false;
        k++;
    }
    // S-strings -- one of them equal to size of the string ==> the node may be complete 
    /*for(int i=0; i < this->inst->m; ++i)
        if(this->inst->S[i].size() <= pl[i])
            return true;*/

    return true;
}

std::vector<int> Node::export_solution()
{
    
    std::vector<int> solution;
    int next_in_s0 =  std::get<0>(this->position)[0];
    if(next_in_s0 == 0) 
        return solution;
    
    int add_letter = this->inst->S[0][next_in_s0 - 1];
    Node* node = this->parent; 
    if(node->l_v + 1 == this->l_v)
       solution.push_back(add_letter);

    next_in_s0 = std::get<0>(node->position)[0]; 

    while(next_in_s0 > 0)
    {
        add_letter = node->inst->S[0][next_in_s0 - 1];
        int l_v_child = node->l_v;
        node = node->parent;
        next_in_s0 = std::get<0>(node->position)[0]; // read the parent node 

        if(node->l_v + 1 == l_v_child )
           solution.push_back( add_letter );

    }
      
    // Reverse the vector using std::reverse
    std::reverse(solution.begin(), solution.end());
    return solution;
}

 // Overload less-than operator for priority_queue
bool Node::operator>(const Node *other)
{
        // Change the comparison logic based on your requirements
        if(this->f_value == other->f_value)
           return this->l_v > other->l_v;
           
        return this->f_value > other->f_value;
}

void Node::print()
{

    std::cout << "Print node information ... \n";
    std::vector<int> pL = std::get<0>(this->position);
    std::vector<int> pleft = std::get<1>(this->position);
    std::vector<int> rleft = std::get<2>(this->position);
    
    std::cout << "  Position vectors within S: ";
    for(int pli: pL)
        std::cout << pli << " ";
    //std::cout << "\n";

    std::cout << "  Position vectors within P: ";

    for(int plefti: pleft)
        std::cout << plefti << " ";
    //std::cout << "\n";   

    std::cout << " Position vectors within R: ";

    for(int rlefti: rleft)
        std::cout << rlefti << " ";
    std::cout << "\n";

    std::cout <<" H-value for the node: " << this->heuristic_value <<  " l_v: " << this->l_v << std::endl;
    std::cout <<" f-value: " << this->f_value << std::endl;

}
 
