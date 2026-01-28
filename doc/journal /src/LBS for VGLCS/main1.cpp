#include "Instance.h"
#include "BeamSearch.h"
#include "globals.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <string>
  
int main(int argc, char* argv[]) { //TODO: update
   
    MLP* neural_network = new MLP();

    // ---------------- Parse CLI ----------------
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // input / output
        if ((arg == "--i" || arg == "-i") && i + 1 < argc) {
            input_path = argv[++i];
        }
        else if ((arg == "--o" || arg == "-o") && i + 1 < argc) {
            outpathname = argv[++i];
        }

        // beam search parameters
        else if ((arg == "--beam_width" || arg == "-b") && i + 1 < argc) {
             beam_width = std::stoi(argv[++i]);
        }
        
        else if ((arg == "--heuristic" || arg == "-h") && i + 1 < argc) {
             heuristic = parseHeuristic(argv[++i]);  
        }
        
        else if ((arg == "--max_iters" || arg == "-m") && i + 1 < argc) {
             max_iters = std::stoi(argv[++i]);
        }
        
        else if ((arg == "--time_limit" || arg == "-t") && i + 1 < argc) {
            time_limit_sec = std::stoi(argv[++i]);
        }
        else if ((arg == "--number_of_roots" || arg == "-n") && i + 1 < argc) {
            number_of_roots = std::stoi(argv[++i]);
        }
        else {
            std::cerr << "Unknown or incomplete argument: " << arg << "\n";
            return 1;
        }
    }

    // ---------------- Validate ----------------
    if (input_path.empty()) {
        std::cerr << "Error: input file not specified (--i <file>)\n";
        return 1;
    }

    // ---------------- Select output stream ----------------
    std::ostream* out = &std::cout;
    std::ofstream fout;

    if (!outpathname.empty()) {
        fout.open(outpathname);
        if (!fout) {
            std::cerr << "Cannot open output file: " << outpathname << "\n";
            return 1;
        }
        out = &fout;
    }

    // ---------------- Echo parameters ----------------
    *out << "BS parameters:\n";
    *out << " input       = " << input_path << "\n";
    *out << " beam_width  = " << beam_width << "\n";
    *out << " heuristic   = " << heuristicToString(heuristic) << "\n";
    *out << " max_iters   = " << max_iters << "\n";
    *out << " time_limit  = " << time_limit_sec << " s\n\n";

    // ---------------- Load instance ----------------
    Instance inst = Instance::loadFromFile(input_path);
    
    std::cout << "=== Loaded instance ===\n";
    inst.print(std::cout);

    // ---------------- Run Beam Search ----------------
    std::cout << "Now run the BS (forward search) " << std::endl;
    BeamSearch::Result res = BeamSearch::run_forward_backward_BS(
        &inst,
        true,
        beam_width,
        heuristic,
        time_limit_sec
    );

    // ---------------- Output result ----------------
    /*out << "=== Beam Search Result ===\n";
    *out << "Best sequence: " << res.best_seq << "\n";
    *out << "Length       : " << res.best_seq.size() << "\n";
    *out << "Runtime (s)  : " << res.runtime << "\n";

    // check feasibility
    bool feasible = check_feasibility(res.steps, inst.gaps, true);
    *out << "Feasible     : " << (feasible ? "YES" : "NO") << "\n";
    
    *out << "Steps:\n";
    for (const auto& step : res.steps) {
        *out <<  "( ";
        for (int p : step)
            *out  << p  <<    " ";
        *out << ") ";
    }*/
    
    //std::cout << "=============================================================================" << std::endl;
    //std::cout << "... and now the backwards BS: " << res.steps[res.steps.size()-1] <<  std::endl;
    /*Node* start_backward = new Node( res.steps[res.steps.size()-1], "", nullptr);
    BeamSearch::Result res1 = BeamSearch::run_forward_backward_BS(
        &inst,
        false, // backward BS
        beam_width,
        heuristic,
        time_limit_sec, 
        {start_backward}
    );
    
    *out << "\n Steps backward :\n";
    for (const auto& step : res1.steps) {
        *out <<  "( ";
        for (int p : step)
            *out  << p  <<    " ";
        *out << ") ";
    }
    *out << "\nBS backward sequence: " << res1.best_seq << "\n";
    
    bool feas = check_feasibility(res1.steps, inst.gaps, true);
    std::cout << "Feasible     : " << (feas ? "YES" : "NO") << "\n";
    
    */
    // IMSBS algorithm
    BeamSearch::Result res_imsbs = BeamSearch::imsbs(&inst, beam_width, 10, 
                                   heuristic, HeuristicType::H5, number_of_roots, 
                                   max_iters, time_limit_sec);
        
    // ---------------- Output result ---------------------
    *out << "=== IMSBS ===\n";  
    *out << "Best sequence: " << res_imsbs.best_seq << "\n";
    *out << "Length       : " << res_imsbs.best_seq.size() << "\n";
    *out << "Runtime (s)  : " << res_imsbs.runtime << "\n";

    // check feasibility
    bool feasible2 = check_feasibility(res_imsbs.steps, inst.gaps, true);
    *out << "Feasible     : " << (feasible2 ? "YES" : "NO") << "\n";
    
    *out << "Steps:\n";
    for (const auto& step : res_imsbs.steps) {
        *out <<  "( ";
        for (int p : step)
            *out  << p  <<    " ";
        *out << ") ";
    }
    
    return 0;
}



