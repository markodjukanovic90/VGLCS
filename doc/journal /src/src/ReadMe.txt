Compilation: In order to compile the program, just write "make" and hit Enter

#############################################################################################################################################################################

Parameters for solving an instance via the learning BS:

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

#############################################################################################################################################################################

Example of a BS solving call (requires having the weights in a file "weights.txt", in the same format as the outputted by the training process): 

#Execute an instance from the benchmark set RANDOM:
./main -i 3_200_3_2_4_0.txt -weights -hidden_layers 3 -units 10 10 5 -o out-g15.txt -b 1000 -t 10 -activation_function 3 -feature_configuration 3

#Execute an instance from the benchmark set ABSTRACT:
./main -i 3_200_3_2_4_0.txt -weights -hidden_layers 3 -units 10 10 10 -o out-g15.txt -b 1000 -t 10 -activation_function 3 -feature_configuration 3
