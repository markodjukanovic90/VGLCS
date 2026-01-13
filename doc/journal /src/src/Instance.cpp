#include "Instance.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <algorithm> // for copy() and assign()  
#include <iterator> // for back_inserter

 Instance::Instance(std::string& path)
{
    this->file_name = path;
    
    // read from data file 
    std::ifstream inputFile(path);  

    // Check if the file is open  
    if (!inputFile.is_open()) {
        std::cerr << "Error opening the file." << std::endl;
        assert(true==false);
        //return 1; // Return an error code
    }

    // Read lines from the file
    std::string line;
    bool leading_line = true;
    std::vector<std::string> strings;
    int num_map = 0;
    int ind=0;
    while (std::getline(inputFile, line)) {
        // Create a stringstream to parse the line
        std::istringstream ss(line);
        if(leading_line)
        {
            // Variables to store the two numbers
            int m, sigma, p, r;

            // Read the two numbers from the line
            if (ss >> m >> sigma >> p >> r)  {
                // Add the pair to the vector
                this->m = m;
                this->Sigma = sigma;
                this->p = p;
                this->r = r;
            } 
            else {
                std::cerr << "Error parsing line: " << line << std::endl;
             }
             leading_line = false; 

        }else{

             std::string si;
             // Read the number and string from the line
             int length;
             if (ss >> length >> si) 
             {
                //std::cout << "\ns=" <<  si.c_str() << std::endl;
                for(auto c : si)
                {     
                    if(map_char_to_int.find(c) == map_char_to_int.end())
                    {
                        map_char_to_int.insert({ c, num_map  });
                        num_map++;
                    }
                }
                std::cout << "s_" << (ind) << " " << si << std::endl;
                ind++;
                strings.push_back(si);

              } else{
                    std::cerr << "Error parsing line: " << line << std::endl;
            }
        }
    }
    // Close the file
    inputFile.close();
    // Convert strings into vectors of integers 
    for(int i = 0; i < this->m; ++i)
    {
        std::vector<int> si_int;
        for(auto chr : strings[i])
        {    
            assert(chr>=0);
            si_int.push_back(map_char_to_int[chr]);
        }
        
        this->S.push_back(si_int);
    }
    // P-strings
    for(int i = this->m; i < this->m + this->p; ++i)
    {
        std::vector<int> si_int;
        for(auto chr : strings[i])
            si_int.push_back(map_char_to_int[chr]);
        this->P.push_back(si_int);
    }
    // R-strings
    for(int i = this->m + this->p ; i < this->m + this->p + this->r; ++i)
    {
        std::vector<int> si_int;
        for(auto chr : strings[i])
            si_int.push_back(map_char_to_int[chr]);
        
        this->R.push_back(si_int);
    }
    // int2char -- reverse 
    for(const auto& [key, val] : map_char_to_int)    
        this->int2char.insert({val, key}); 

    //std::cout << "FILL IN NECESSARY DATA STRUCTURES ..."<< std::endl;
    this->fill_in_data_structures();
}

void Instance::UB1()
{
     
     //std::cout << "calculate ub for the instance " << std::endl;
     int ub = 0;
     for(int l = 0; l < this->Sigma; ++l)
     {
        int s_let = 10000;
        for(int i = 0 ; i < this->m; ++i)
            s_let = std::min(s_let, this->occurances_string_pos_char[l][i][ 0 ]);
        ub += s_let;
     }
     this->UB = ub;
}

std::vector<std::vector<int>> Instance:: M_score_i(int k)
{
    int kxpres = k+1;
    std::vector<std::vector<int>> M((int)this->S[k].size()+1, std::vector(this->S[kxpres].size()+1, 0 ) );

    std::vector<int> s_k = this->S[k];
    std::vector<int> s_k_plus1 = this->S[kxpres];

    for(int i = 1; i <= (int)s_k.size(); ++i)
    {
        for(int j = 1; j <= (int)s_k_plus1.size(); ++j)
        {
            
            if(s_k[ s_k.size() - i  ] == s_k_plus1[ (int)s_k_plus1.size() - j  ]  )
                M[ (int)s_k.size() - i ][ (int) s_k_plus1.size() - j ] = 1 + M[(int)s_k.size() - i + 1][ s_k_plus1.size() - j + 1];
            else
                M[ (int)s_k.size() - i ][ (int) s_k_plus1.size() - j ] 
                = std::max(M[ (int) s_k.size() - i + 1][ (int) s_k_plus1.size() - j], M[ (int) s_k.size() - i][ (int) s_k_plus1.size() - j + 1]);
        }
    }
    return M;
}

void Instance::Mousavi_probability_matrix()
{
      
    int n=0;
    for(int i=0;i < this->m; ++i)
        if((int)this->S[i].size() > n)
            n=(int)this->S[i].size();
    
    //create the P matrix of probabilities:
    for(int i=0;i<=n; ++i)
    {
        std::vector<double> p_i(n+1, 0.0);
        Prob.push_back(p_i); 
        p_i.clear();
    }
    // trivial value i=0:
    for(int j=0;j<=n; ++j)
        Prob[0][j] = 1.0;
    // trivial value j=0 (i>0):
    for(int i=1;i<=n; ++i)
        Prob[i][0]=0.0;
    
    for(int i=1;i<=n; ++i)
    {
        for(int j=1;j<=n; ++j)
        {
             if(i>j)  
                Prob[i][j] = 0.0;
             else
                Prob[i][j] = 1.0/this->Sigma*Prob[i-1][j-1] + (this->Sigma-1.0) / this->Sigma *Prob[i][j-1];
        }
    }
}

void Instance::fill_in_data_structures()
{

    for(int a = 0; a < this->Sigma; ++a)
    {
        std::vector<std::vector<int>> occur_a_all_s;
        for(int i = 0; i < this->m; ++i)
        {      
            std::vector<int> pos_occur_a_in_si;
            int num_occurance = 0;
            for(int j = (int) this->S[i].size()-1; j >=0; j--)
            {

                if(this->S[i][j] == a)  
                    num_occurance++;
                pos_occur_a_in_si.insert(pos_occur_a_in_si.begin(), num_occurance);
            
            }   
            occur_a_all_s.push_back(pos_occur_a_in_si);
        }
        this->occurances_string_pos_char.push_back(occur_a_all_s);
    }

    // next_char_occurance_in_strings[a, i, j)
    for(int a = 0; a < this->Sigma; ++a)
    {
        std::vector<std::vector<int>> next_occur_a_all_s;
        for(int i = 0; i < this->m; ++i)
        {
            int next_pos_of_occur = (int) this->S[i].size();
            std::vector<int> next__occur_a_in_si( this->S[i].size(), next_pos_of_occur);

            for(int j = (int) this->S[i].size() -1; j >= 0;--j)
            {

                if(this->S[i][j] == a) 
                    next_pos_of_occur   = j;
                next__occur_a_in_si [j] = next_pos_of_occur;
            }
            next_occur_a_all_s.push_back(next__occur_a_in_si);
        }
         this->next_char_occurance_in_strings.push_back(next_occur_a_all_s);
    }

    // remaining_patern_suffix_pos -- a max index of si for which Pj[px, |P|] can be imputed in si[1, |si|]
    for(int i = 0; i < this->m; ++i)
    {
         std::vector<std::vector<int>> embedding_all_pj_into_si;
         for(int j = 0; j<this->p;j++)
         {  
            std::vector<int> embeding_pj_into_si((int)this->P[j].size(), -1);
            int max_index_emeding_remaining_sufix_pj = (int) this->P[j].size()-1;
            //int curr_pos_si = this->S[i].size()-1;

            for(int its = (int) this->S[i].size()-1; its >= 0; --its)
            {

                if(max_index_emeding_remaining_sufix_pj>=0 && this->P[j][max_index_emeding_remaining_sufix_pj] == this->S[i][its])
                {    
                    embeding_pj_into_si[max_index_emeding_remaining_sufix_pj] = its;
                    max_index_emeding_remaining_sufix_pj--; // previous sufix 
                }
                //curr_pos_si--;
            }
            embedding_all_pj_into_si.push_back(embeding_pj_into_si);
         }
         this->remaining_patern_suffix_pos.push_back(embedding_all_pj_into_si);
    }

    // fill in M-score matrices:
    for(int i=0; i < this->m-1; ++i)
    {
          std::vector<std::vector<int>> M_i = this->M_score_i(i);
          M_scores.push_back(M_i);
    }
    // calculate UB of the instance
    this->UB1();
    // fill in the Mousavi's matrix:
    this->Mousavi_probability_matrix();

}

void Instance::print_info()
{
    
    std::cout << "Information for the instance ... " << std::endl;
    for(int i = 0; i < (int)(this->S).size(); ++i)
    {
        std::cout << "s_" << std::to_string(i) << ": ";
        for(auto sij : this->S[i])
            std::cout << sij <<" ";
        std::cout << "\n" << std::endl;
    }
    // Read P 
    std::cout << "\nInformation for the P-strings ... ";
    for(int i = 0; i < (int)(this->P).size(); ++i)
    {
        std::cout << "p_" << std::to_string(i) << ": ";
        for(auto pij : this->P[i])
            std::cout << pij <<" ";
        std::cout << "\n" << std::endl;
    }
    std::cout << "\nInformation for the R-strings ... ";
    // read R strings -- the restricted input strings
    for(int i = 0; i < (int)(this->R).size(); ++i)
    {
        std::cout << "r" << std::to_string(i) << ": ";
        for(auto rij : this->R[i])
            std::cout << rij << " ";
        std::cout << "\n" << std::endl;
    }
    // this->occurances_string_pos_char
    std::cout << "occurances_string_pos_char[0][0][j]" << std::endl;
    for(int j=0; j < (int)this->S[0].size(); ++j)
        std::cout << this->occurances_string_pos_char[0][0][j] << " ";

    // this->remaining_patern_suffix_pos
    std::cout << "\n Embed -- remaining_patern_suffix_pos for s0 and p0" << std::endl;
     
    for(int j=0; j < (int)this->P[0].size(); ++j)
        std::cout << this->remaining_patern_suffix_pos[0][0][j]<< " " ;
 
    std::cout <<  "\nUB: " << this->UB << std::endl;

}

Instance::~Instance()
{
    for(auto& si : this->S)
        si.clear(); 
    
    for(auto& pj : this->P)
        pj.clear();
    
    for(auto& rk: this->R)
        rk.clear();
   
}
