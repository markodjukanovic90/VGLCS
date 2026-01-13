// https://www.quantstart.com/articles/Function-Objects-Functors-in-C-Part-1/ --> Function Pointers
#pragma once

#include <iostream>
#include <vector>
#include <algorithm> 
#include <cassert>
#include "Instance.h"

// Abstract base class                                                                                                                                                                                                  
class HeuristicFunction {
public:

  HeuristicFunction() {};
  virtual double operator() (std::vector<int>& pL, std::vector<int>& left, std::vector<int>& rleft, int letter, Instance* inst) = 0;
};

// Greedy function                                                                                                                                                                                                      
class Greedy : public HeuristicFunction {
public:
  Greedy() {};
  virtual double operator() (std::vector<int>& pL, std::vector<int>& pleft, std::vector<int>& rleft, int letter, Instance* inst) {   

       double greedy = 0.0;
       for(int i=0; i < inst->m;++i)
       {

           int next_a = inst->next_char_occurance_in_strings[letter][i][pL[i]];
           int skiped_for_a = next_a - pL[i] +1;
           int remaining = inst->S[i].size() - pL[i] +1;
           double score_i = ((double) skiped_for_a) /  remaining;
           
           greedy += score_i;
       }
       // include  P-strings 
       for(int j = 0; j < inst->p; ++j)
       {
           if(inst->P[ j ][ pleft[j] ] == letter)
               greedy += 1.0 / ( inst->P[j].size() - pleft[j] - 1 ) * inst->Sigma;
           else
               greedy += 1.0 / ( inst->P[j].size() - pleft[j] ) * inst->Sigma;
       }
       
       return greedy;
   }
};

class UB1 : public HeuristicFunction
{
     
  public:
     UB1(){};

     virtual double operator()(std::vector<int>& pL, std::vector<int>& pleft, std::vector<int>& rleft, int letter, Instance* inst) {   
             
            double ub = 0.0;
            for(int l = 0; l < inst->Sigma; ++l)
            {
                int s_min_l = 10000;
                for(int i = 0 ; i < inst->m; ++i)
                {
                     int l_a = inst->occurances_string_pos_char[l][i][ pL[i] ];
                     s_min_l = std::min(s_min_l, l_a);
                }
                ub += s_min_l;
            }
            return ub;
     }
};

class UB2 : public HeuristicFunction
{

  public:
     UB2(){};

     virtual double operator()(std::vector<int>& pL, std::vector<int>& pleft, std::vector<int>& rleft, int letter, Instance* inst)
     {

          double ub2 = 100000;
          for(int i = 0; i < inst->m-1; ++i)
          {
              assert(pL[i] <= (int)inst->S[i].size());
              if(ub2 > inst->M_scores[i][pL[i]][pL[i+1]])
                 ub2 = inst->M_scores[i][pL[i]][pL[i+1]];

          }

          return ub2;
     }
};

inline double binary_op(std::vector<int>& pL, std::vector<int>& pleft, std::vector<int>& rleft, int letter, Instance* ins, HeuristicFunction* bin_func) {
  return (*bin_func)( pL, pleft,  rleft, letter, ins);
}

class UB : public HeuristicFunction
{
    public:
    UB(){};

    virtual double operator()(std::vector<int>& pL, std::vector<int>& pleft, std::vector<int>& rleft, int letter, Instance* inst)
    {
         
         HeuristicFunction* hf1 = new UB1();
         HeuristicFunction* hf2 = new UB2();
         
         double val1 = binary_op(pL, pleft, rleft, letter, inst, hf1);  
         double val2 = binary_op(pL, pleft, rleft, letter, inst, hf2);
         
         if(val1 < val2)
            return val1;
         else
            return val2;
     } 
};

class Probability : public HeuristicFunction
{
     //Constructor:
     public:
     int k=0;
     Probability(int kx){ k = kx; };

     virtual double operator()(std::vector<int>& pL, std::vector<int>& pleft, std::vector<int>& rleft, int letter, Instance* inst) {   
            //mstd::cout <<  "k----> " << std::endl;
            double h_val = 1.0;
            for(int i = 0; i < inst->m; ++i)
                h_val *= inst->Prob[this->k][inst->S[i].size() - pL[i] + 1];
            //for(int l = 0; l < inst->r; ++l)
            //    h_val *= (1.0 - inst->Prob[inst->R[l].size()-rleft[l]+1][this->k]);
            /*int p_i = 10000; 
            int index=0;

            for(int rl_i: rleft)
            {
                if(p_min > inst->R[index].size() - rl_i +1)
                  
                   p_i  = inst->R[index].size() - rl_i +1;
                   h_val *= (1- inst->Prob[p_i][ this->k ]);
                   
                index++;
            }*/
            //std::cout << p_min << std::endl;
            //h_val *= (1- inst->Prob[p_min][ this->k ]);
            
            return h_val;
     }    
};
class R_min : public HeuristicFunction
{
     //Constructor:
     public:
     int k=0;
     R_min(){ };

     virtual double operator()(std::vector<int>& pL, std::vector<int>& pleft, std::vector<int>& rleft, int letter, Instance* inst) {   

            int p_min = 10000; int index=0;

            for(int rl_i: rleft)
            {
                if(p_min > (int)inst->R[index].size() - rl_i +1)
                   p_min  = inst->R[index].size() - rl_i +1;
                   
                index++;
            }
            
            return (double)p_min;
     }    
};

