
#include<string>
#include<vector>
#include <unordered_map>
#include "Instance.h"

/* Hashing a vector of integers for APPROX2 algorithm */
struct VectorHasher {
    template <typename T>
    std::size_t operator()(const std::vector<T>& vec) const {
        // Combine the hash values of individual elements in the vector
        std::size_t hash = 0;
        for (const T& element : vec) {
            hash ^= std::hash<T>()(element) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

/* end hashing for  APPROX algorithm */

class Algorithm
{

    public:
        
        Instance *inst = nullptr;
        int number_algorithm = 0; // which of the algorithm to execute
        double timelimit = 600.0;
        /* stats values*/
        int num_iterations = 0;
        std::vector<int> solution;
        double running_time = 0.0;
        int beta=0;
        double time_best_solution_found = 0.0;
        int num_iteration_best_solution_found = 0;
        int num_Q_nodes = 0;// A* search stats
        int num_N_nodes = 0;// A* search stats
        int additional_info=0;
        /* a data structure for Approx2 */
    
    public:

        Algorithm(int alg, Instance *instance, double t_lim, int beta);

        void run();
        /** Algorithm types **/
        void Greedy(double t_lim, int  beta);
        void BS(double t_lim, int beta);
        void A_star(double t_lim);
        //Approximation algorithms
        void initialize(auto& LCS, std::vector<int16_t>& pos);
        void DP_LCS(auto& LCS, std::vector<int16_t>& pos);
        void extract_lcs_solution(auto& LCS, std::vector<int16_t>& pos);
        void Approximation1();
        void Approximation2();
        // DP algorithm for Restricted LCS
        void extract_DP_RLCS_solution(auto& RLCS, std::vector<int>& pos);
        void DP();
        /** general rlcs **/
        //void initialize_general_rlcs(auto& RLCS, std::vector<int>& pos);
        void extract_solution_general_rlcs(auto& RLCS, std::vector<int16_t>& final_state);
        int DP_general_recursion(auto& RLCS, std::vector<int16_t>& final_vector);
        void DP_general_rlcs_run(); // ecxecute the DP recursion
        /** Destructor of the class **/
        bool validate_solution();
        void save_in_file(const std::string&  outpath);
        ~Algorithm();
};