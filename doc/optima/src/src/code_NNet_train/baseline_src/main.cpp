#include <iostream>
#include <fstream>
#include <sstream>
//#include "Instance.h"
#include "Node.h"
#include "Algorithm.h"
#include "global_variable.h"

using namespace std;

int main(int args, char* argv [])
{

    // Check if the correct number of command line arguments is provided
    /*if (args != 13) {
        std::cerr << "Usage: " << argv[0] << " -t <time(double)> -i <input_file>  -o <output-file> -a <algorithm_type> -b <beta> -h <heuristic>" << std::endl;
        return 1; // Return an error code
    }*/
    // accept the terminal values:
    double time_limit=0.0;
    int algorithm, beta = 0;
    
    /* write all terminal-line-params into a string */
    for (int i = 0; i < args; ++i) {
        out_terminal_string += argv[i];
        if (i < args - 1) {
            out_terminal_string += ' '; // Add space between arguments
        }
    }

    /* read terminal line params values */
    for (int i = 1; i < args; ++i) {
        std::string arg = argv[i];

        if (arg == "-t" && i + 1 < args) {
            try {
                time_limit = std::stod(argv[++i]); // Convert the next argument to a double
            } catch (const std::invalid_argument& e) {
                std::cerr << "Error: Invalid argument for -t option. " << e.what() << std::endl;
                return 1; // Return an error code
            } catch (const std::out_of_range& e) {
                std::cerr << "Out of range error: " << e.what() << std::endl;
                return 1; // Return an error code
            }
        } else if (arg == "-i" && i + 1 < args) {
            filename = argv[++i]; // Assign the next argument to the input file
        } else if (arg == "-o" && i + 1 < args) {
            outpathname = argv[++i]; // Assign the next argument to the input file
        } else if (arg == "-a" && i + 1 < args) {
            algorithm = atoi(argv[++i]); // Assign the next argument to the algorithm type
        } else if (arg == "-b" && i + 1 < args) {
            beta = atoi(argv[++i]); // Assign the next argument to the beta size in BS algorithm  
        } else if (arg == "-h" && i + 1 < args) {
            h_heuristic = atoi(argv[++i]); // Assign the next argument to heuristic utilized within a BS
        } 
          else if(arg == "-pctext" && i+1 <= args){
            pct_v_ext = atof(argv[++i]);
        }
        else {
            std::cerr << "Error: Invalid option or missing argument." << std::endl;
            return 1; // Return an error code
        }
    }  
    // if everything is fine, proceed further --
    Instance *inst = new Instance(filename);
    //srand((unsigned) time(NULL));
    Algorithm* alg = new Algorithm(algorithm, inst, time_limit, beta);
    alg->run();

    return 0;
}