#pragma once

#include <vector>
#include <map>
#include <string>
#include <string.h>


class Instance
{
  
   public:

       std::string file_name;
       int m;
       int Sigma;
       int p;
       int r;
       int UB;

        std::vector<std::vector<int>> S;
        std::vector<std::vector<int>> P;
        std::vector<std::vector<int>> R;
        /** preprocessing structures **/
        std::map<char, int> map_char_to_int;
        std::map<int, char> int2char;
        std::vector<std::vector<std::vector<int>>> occurances_string_pos_char; // occurances_string_pos_char[char,i, j] --- number of occurances of char CHAR in s[i][j, |si|] 
        std::vector<std::vector<std::vector<int>>> next_char_occurance_in_strings; // next_char_occurance_in_strings[char, i, j] -- the position of next occurances of char CHAR >= j
        std::vector<std::vector<std::vector<int>>> remaining_patern_suffix_pos; // remaining_patern_suffix_pos[i,j, px] -- a max index of si for which Pj[px, |P|] can be imputed in si[i, |si|]
        std::vector<std::vector<std::vector<int>>> M_scores; // i-th position -- LCS(s_i^rev, s_j^rev)
        std::vector<std::vector<double>> Prob;
   
   public:

        Instance(std::string& path);        
        void print_info(); 
        void fill_in_data_structures();
        std::vector<std::vector<int>> M_score_i(int i);
        void Mousavi_probability_matrix();
        void UB1(); // TODO
        ~Instance();

};