import os

train_files = []
validate_files = []

TRAIN_DIR = "train/"
VALIDATE_DIR = "validate/"

# -------- TRAIN FILES --------
for f in os.listdir(TRAIN_DIR):
    full_path = os.path.join(TRAIN_DIR, f)
    if os.path.isfile(full_path):
        train_files.append(full_path)

# -------- VALIDATION FILES --------
for f in os.listdir(VALIDATE_DIR):
    full_path = os.path.join(VALIDATE_DIR, f)
    if os.path.isfile(full_path):
        validate_files.append(full_path)

# Optional: sort for reproducibility
train_files.sort()
validate_files.sort()

print("Validation files:")
print(validate_files)

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
cmd = (
    f"{beamsearch_exe} "
    f"-train {' -train '.join(train_files)} "
    f" -validation {' -validation '.join(validate_files)} "
    f"-hidden_layers {hidden_layers} "
    f"-units {' '.join(str(u) for u in units)} "
    f"-weight_limit {weight_limit} "
    f"-training_beam_width {training_beam_width} "
    f"-training_time_limit {training_time_limit} "
    f"-activation_function {activation_function} "
    f"-feature_configuration {feature_configuration} "
    f"-ga_configuration {ga_configuration}"
)

print("\nCommand:")
print(cmd)

