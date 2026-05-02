#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <unordered_map>
#include <map>
#include <queue>
#include <chrono> // measure a time of algorithms execution
#include <algorithm> // for std::sort  
#include <fstream>
#include <set>
#include <numeric>
//#include "Node.h"
//#include "globals.h"  
//#include "nnet.h"
#include "nnet.h"

class MLP;

double BS(double t_lim, int beta, Instance* inst, MLP& neural_network, bool training);