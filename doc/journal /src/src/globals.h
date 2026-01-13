#ifndef GLOBAL_VARIABLE_H
#define GLOBAL_VARIABLE_H

#include <string>

extern int num_features;
extern int h_heuristic;
extern double pct_v_ext;
extern std::string outpathname;
extern std::string out_terminal_string;
extern std::string filename;
extern int activation_function;
extern int feature_config;
extern int population_size;
extern int n_elites;
extern int n_mutants;
extern int ga_config;
extern double elite_inheritance_probability;
extern bool differentiate_ties;
#endif // GLOBAL_VARIABLE_H