#include <iostream>
#include <fstream>
#include <sstream>
#include "beam_search.h"
#include "globals.h"


using namespace std;
MLP neural_network;
bool training = false;
int beam_width = 10;
double time_limit = 10;

void read_parameters(int argc,char** argv){
    int iarg = 1;
    int hidden_layers = 0;
    vector<int> units;
    while (iarg < argc) {
        if (strcmp(argv[iarg], "-train") == 0) {
            if(not training) training = true;
            string name = argv[++iarg];
            neural_network.training_instances.push_back(Instance(name));
        }
        else if (strcmp(argv[iarg], "-validation") == 0){
            string name = argv[++iarg];
            neural_network.validation_instances.push_back(Instance(name));
        } 
        else if (strcmp(argv[iarg], "-weight_limit") == 0) neural_network.weight_limit = atoi(argv[++iarg]);
        else if (strcmp(argv[iarg], "-training_beam_width") == 0) neural_network.training_beam_width = atoi(argv[++iarg]);
        else if (strcmp(argv[iarg],"-training_time_limit") == 0) neural_network.training_time_limit = atof(argv[++iarg]);
        else if (strcmp(argv[iarg], "-hidden_layers") == 0) hidden_layers = atoi(argv[++iarg]);
        else if (strcmp(argv[iarg], "-units") == 0){
        for(int i = 0; i < hidden_layers; ++i)
            units.push_back(atoi(argv[++iarg]));
        }
        else if (strcmp(argv[iarg], "-t") == 0){
            time_limit = atoi(argv[++iarg]);
        }
        else if (strcmp(argv[iarg], "-i") == 0){
            filename = argv[++iarg];
        } 
        else if (strcmp(argv[iarg], "-o") == 0){
            outpathname = argv[++iarg];
        }
        else if (strcmp(argv[iarg], "-b") == 0){
            beam_width = atoi(argv[++iarg]);  
        } else if (strcmp(argv[iarg], "-h") == 0) {
            h_heuristic = atoi(argv[++iarg]); 
        } 
          else if(strcmp(argv[iarg], "-pctext") == 0){
            pct_v_ext = atof(argv[++iarg]);
        }
        else if(strcmp(argv[iarg], "-activation_function") == 0){
            activation_function = atoi(argv[++iarg]);
        }
        else if(strcmp(argv[iarg], "-feature_configuration") == 0){
            feature_config = atoi(argv[++iarg]);
            if(feature_config == 1) num_features = 9; //p^{L,v}, l^v and length of partial sol 
            else if(feature_config == 2) num_features = 10; // plus alphabet size 
            else if(feature_config == 3) num_features = 12; //plus num of input and restricted strings
            else if(feature_config == 4) num_features = 14; //plus num of and length of input and restricted strings
        }
        else if (strcmp(argv[iarg], "-ga_configuration") == 0){ //1: rkga, 2: brkga, 3: lexicase sel.
            ga_config = atoi(argv[++iarg]);
        }
        else if(strcmp(argv[iarg], "-population_size") == 0){
            population_size = atoi(argv[++iarg]);
        }
        else if(strcmp(argv[iarg], "-n_elites") == 0){
            n_elites = atoi(argv[++iarg]);
        }
        else if(strcmp(argv[iarg], "-n_mutants") == 0){
            n_mutants = atoi(argv[++iarg]);
        }
        else if(strcmp(argv[iarg], "-rho") == 0){
            elite_inheritance_probability = atof(argv[++iarg]);
        }
        else if(strcmp(argv[iarg], "-differentiate_ties") == 0){
            differentiate_ties = true;
        }
        iarg++;
    }
    //set up neural network

    neural_network.units_per_layer.push_back(num_features); //input layer
    for(int i = 0; i < hidden_layers; ++i){ //inner layers
        neural_network.units_per_layer.push_back(units[i]);
    }
    neural_network.units_per_layer.push_back(1); //output layer
    if(not training){
        ifstream weights_file("weights.txt", std::ios::in);
        if(!weights_file.is_open()){
            cout << "Couldn't open the weights file" << endl;
            exit(-1);
        }
        vector<double> temporal_weights;
        double num = 0.0;  
        while(weights_file >> num) temporal_weights.push_back(num);
        neural_network.store_weights(temporal_weights);
    }

    if(feature_config != 1 and feature_config != 2 and feature_config != 3 and feature_config != 4){
        cout << "WARNING: feature configuration not specified. Using default value of one" << endl;
        feature_config = 1;
    }
    if(ga_config != 1 and ga_config != 2 and ga_config != 3 and training){
        cout << "WARNING: training ga configuration not specified. Using default value of one (rkga)" << endl;
        ga_config = 1;
    }
    if(neural_network.weight_limit == 0 and training){
        cout << "WARNING: weight limit not specified. Using default value of one" << endl;
        neural_network.weight_limit = 1;
    }
    if(neural_network.units_per_layer.size() == 0){
        cout << "ERROR: the shape of the neural network needs to be explicited " << endl;
        exit(-1);
    }
    if(activation_function!= 1 and activation_function != 2 and activation_function != 3){
        cout << "WARNING: using no activation function, explicit one with -activation_function {1:tanh , 2:relu, 3:sigmoid}" << endl;
    }
}

int main( int argc, char **argv ) {

    read_parameters(argc, argv);
    if(training){
        std::cout << std::setprecision(10) << std::fixed;
        vector<double> res_weights = neural_network.Train();
        
        // write best weights inside weights file
        fstream weights_file;
        weights_file.open("last_weights.txt",ios_base::out);
        for(double weight : res_weights)
            weights_file << weight << " ";
        
        weights_file.close();
    }
    else {
        Instance* inst = new Instance(filename);
        BS(time_limit, beam_width, inst, neural_network, false);
        delete inst;
    }
    return 0;
}
