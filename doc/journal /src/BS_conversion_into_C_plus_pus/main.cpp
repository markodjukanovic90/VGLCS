#include "Instance.h"
#include "Utils.h"
#include <iostream>

int main(int argc, char* argv[]) {

    // Hand-on setup (or parse with argv)
    params.beam_width = 20;
    params.heuristic = parseHeuristic("h5");
    params.max_iters = 5000;

    std::cout << "BS parametri:\n";
    std::cout << " beam_width = " << params.beam_width << "\n";
    std::cout << " heuristic  = " << heuristicToString(params.heuristic) << "\n";
    std::cout << " max_iters  = " << params.max_iters << "\n";

    Instance inst;
    //inst.runBeamSearch();

    return 0;
}
