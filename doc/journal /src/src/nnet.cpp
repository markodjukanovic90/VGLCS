#include "beam_search.h"
#include "globals.h"

unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::default_random_engine generator(seed);
std::uniform_real_distribution<double> standard_distribution_01(0.0,1.0);

// training GA parameters
int training_BS_time_limit = 10;
int training_BS_beta = 100;

Eigen::MatrixXd relu(const Eigen::MatrixXd& x) {
    return x.array().max(0);
}

Eigen::MatrixXd sigmoid(const Eigen::MatrixXd& x) {
    return 1.0 / (1.0 + (-x).array().exp());  
}

void apply_activation_function(Eigen::MatrixXd& x){
    if(activation_function == 1) x = x.array().tanh();
    else if(activation_function == 2) x = relu(x);
    else if(activation_function == 3) x = sigmoid(x);
    
}


int produce_random_integer(int max, double rval) {

    int num = int(double(max) * rval);
    if (num == max) num = num - 1;
    return num;
}

void print_information(double best_ofv, double ctime, int niter, double validation_value){
    
    cout << endl;
    cout <<"-------------------------------------------------------------------------------------------" << endl;
    cout << "best " << best_ofv << " time " << ctime << " niter: " << niter + 1 << " validation value: " << validation_value << endl;
    cout <<"-------------------------------------------------------------------------------------------" << endl;
}
  
MLP::MLP(){}

Eigen::VectorXd MLP::forward(const Eigen::VectorXd& x) {
    Eigen::MatrixXd x_mat = x;
    Eigen::MatrixXd prev = x_mat;
    for(int i = 0; i < (int)units_per_layer.size() - 1; ++i){
        Eigen::MatrixXd y = weight_matrices[i] * prev;
        y = y + bias_vectors[i];
        apply_activation_function(y);
        prev = y;
    }
    return prev;
}
 

void MLP::store_weights(const vector<double>& weights){ //store weights into neural network
    
    weight_matrices.clear();
    bias_vectors.clear(); 
    int stored_weights = 0;
    
    assert(weight_matrices.size() == 0 and bias_vectors.size()==0);
    
    for(int i = 0; i < (int)units_per_layer.size() - 1; ++i){
        //weight matrix from layer i to layer i + 1
        int num_rows = units_per_layer[i + 1];
        int num_cols = units_per_layer[i];
        Eigen::MatrixXd weights_matrix(num_rows, num_cols);
        for (int i1 = 0; i1 < num_rows; ++i1) {
            for(int i2 = 0; i2 < num_cols; ++i2){
                weights_matrix(i1, i2) = weights[stored_weights];
                stored_weights ++;
            }
        }
        weight_matrices.push_back(weights_matrix);
        //bias vector from layer i to layer i + 1
        int num_biases = units_per_layer[i + 1];
        Eigen::MatrixXd bias_vector(num_biases, 1);
        for(int i1 = 0; i1 < num_biases; ++i1){
             bias_vector(i1) = weights[stored_weights];
             stored_weights ++;
        }
        bias_vectors.push_back(bias_vector);
    }
}
