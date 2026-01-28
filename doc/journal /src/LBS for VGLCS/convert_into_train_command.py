train_files = ["train/mglcs_3_500_2_0.txt", "train/mglcs_3_100_4_0.txt"]
validate_files = ["validate/mglcs_10_200_2_1.txt", "validate/mglcs_5_500_4_1.txt"]

beamsearch_exe = "./beamsearch"
hidden_layers = 2
units = [5, 5]
weight_limit = 1
training_beam_width = 10
training_time_limit = 60
activation_function = 1
feature_configuration = 3
ga_configuration = 1

# Build the command string
cmd = f"{beamsearch_exe} -train {' '.join(train_files)} -validation {' '.join(validate_files)} " \
      f"-hidden_layers {hidden_layers} -units {' '.join(str(u) for u in units)} " \
      f"-weight_limit {weight_limit} -training_beam_width {training_beam_width} " \
      f"-training_time_limit {training_time_limit} -activation_function {activation_function} " \
      f"-feature_configuration {feature_configuration} -ga_configuration {ga_configuration}"

print(cmd)
