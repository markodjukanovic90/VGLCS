// globals.cpp

#include "globals.h"

int h_heuristic = 0;
double pct_v_ext = 1.0/3;
std::string outpathname="";
std::string out_terminal_string=""; 
int num_features = 9; //number of features
std::string filename;
int activation_function = 0; // activation function used
int feature_config = 0; // features used
//int ga_config = 0; //type of GA used for training (rkga, brkga, lexicase selection)
//training rkga/brkga parameters
//int population_size = 20;
//int n_elites = 1;
//int n_mutants = 7;
//double elite_inheritance_probability = 0.5; //brkga rho
bool differentiate_ties = false; //whether to use the probabilistic heuristic as a secondary heuristic function
