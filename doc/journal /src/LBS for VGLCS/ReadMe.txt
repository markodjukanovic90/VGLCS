Compilation: In order to compile the program, just write "make" and hit Enter

Parameters for solving an instance:
-i: path to an instance to be solved
-o: path to a file where output information will be stored
-b: beam width
-t: time limit
-hidden_layers: number of hidden layers of the NN guiding the BS
-units: one number for each hidden layer, indicating the hidden units for that layer
-weights: indicates that a "weights.txt" file is provided in the directory of execution, containing the weights to be used for the NN guiding the BS.
-activation_function: indicates the activation function to be used.
1 -> tanh; 2 -> reLU; 3 -> sigmoid
-feature_configuration: indicates the features to be used.
1 -> max, min, std and average of p^{L,v} and l^v and the length of the partial solution associated to node v are used (9 features)
2 -> the alphabet size is added
3 -> the alphabet size is added and the number of input and restricted strings
4 -> the alphabet size is added and the number and length of the input and restricted strings (assumes that every input and restricted string is of the same length)


Parameters for training:
-hidden_layers: number of layers of the NN to be trained
-units: one number for each layer, indicating the hidden units for that layer
-train: paths to training files (use one -train directive for each training instance)
-validation: paths to validation files (use one -validation instance for each validation instance)
-weight_limit: integer value which determines the maximum absolute value allowed for the weights in the training process
-training_beam_width: beam width used in the training process
-training_time_limit: time limit for training
-activation_function: indicates the activation function to be used.
1 -> tanh; 2 -> reLU; 3 -> sigmoid
-feature_configuration: indicates the features to be used.
(same as for experiments)
-ga_configuration: type of ga used for training: 
1 -> rkga used 
2 -> brkga used, specify the elite inheritance probability with -rho 
3 -> rkga with lexicase selection for selecting the elite population is used


Example of a solving call (requires having the weights in a file "weights.txt", in the same format as the outputted by the training process): 

./beamsearch -i sigma-4/mglcs_5_500_4_1.txt -o out-111.txt  -hidden_layers 3 -units 5 5 5 -weights -activation_function 1 -feature_configuration 1

Example of a training call:

./beamsearch -train train/mglcs_3_500_2_0.txt -train train/mglcs_3_100_4_0.txt -validation validate/mglcs_10_200_2_1.txt -validation validate/mglcs_5_500_4_1.txt  -hidden_layers 3 -units 5 5 5 -weight_limit 1 -training_beam_width 10 -training_time_limit 200 -activation_function 3 -feature_configuration 1 -ga_configuration 1
