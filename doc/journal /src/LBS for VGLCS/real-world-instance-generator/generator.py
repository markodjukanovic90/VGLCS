import numpy as np
import random

# --- SantaLucia-inspired dinucleotide weights ---
DINUC_WEIGHT = {
    'AA':1.00,'TT':1.00,'AT':0.88,'TA':0.58,
    'CA':1.45,'TG':1.45,'GT':1.44,'AC':1.44,
    'CT':1.28,'AG':1.28,'GA':1.30,'TC':1.30,
    'CG':2.17,'GC':2.24,'GG':1.84,'CC':1.84
}

ALPHABET = ['A','C','G','T']


# -------------------------------
# 1. Sequence generation (entropy control)
# -------------------------------
def generate_sequence(n, entropy_level=1.0):
    if entropy_level == 1.0:
        probs = [0.25]*4
    else:
        biased = np.array([0.4, 0.1, 0.1, 0.4])  # A,C,G,T
        uniform = np.ones(4)/4
        probs = entropy_level * uniform + (1-entropy_level) * biased
        probs /= probs.sum()
    
    return ''.join(np.random.choice(ALPHABET, size=n, p=probs))


# -------------------------------
# 2. Mutation
# -------------------------------
def mutate(seq, mutation_rate=0.1):
    seq = list(seq)
    for i in range(len(seq)):
        if random.random() < mutation_rate:
            seq[i] = random.choice(ALPHABET)
    return ''.join(seq)


# -------------------------------
# 3. Influence computation
# -------------------------------
def compute_influence(seq, k=5, lambda_decay=0.5):
    """
    Causal influence:
    - dinucleotide contribution: f(k) = f(s_{k-1}, s_k)
    - influence: only from previous positions
    """
    import numpy as np
    
    n = len(seq)
    
    # -------------------------------
    # 1. Causal dinucleotide contribution
    # -------------------------------
    f = np.zeros(n)
    
    for k_pos in range(n):
        if k_pos > 0:
            dinuc = seq[k_pos-1] + seq[k_pos]
            f[k_pos] = DINUC_WEIGHT[dinuc]
        else:
            f[k_pos] = 0.0  # first position has no predecessor
    
    # -------------------------------
    # 2. Causal influence aggregation
    # -------------------------------
    I = np.zeros(n)
    
    for i in range(n):
        for k_pos in range(max(0, i-k), i+1):
            dist = i - k_pos
            influence = np.exp(-lambda_decay * dist)
            I[i] += influence * f[k_pos]
    
    return I


# -------------------------------
# 4. Influence → gap
# -------------------------------
def influence_to_gap(influence, w_min=1, w_max=10, noise=0.0):
    inf = np.array(influence)
    
    # normalize to [0,1]
    inf_norm = (inf - inf.min()) / (inf.max() - inf.min() + 1e-8)
    
    # DIRECT mapping (changed!)
    gaps = w_min + (w_max - w_min) * inf_norm
    
    # optional noise
    if noise > 0:
        gaps += np.random.normal(0, noise, size=len(gaps))
    
    return np.clip(gaps.astype(int), w_min, w_max)


# -------------------------------
# 5. MAIN: generate m sequences
# -------------------------------
def generate_instance_m(
    m=3,
    n=100,
    entropy=1.0,
    mutation_rate=0.1,
    k=5,
    lambda_decay=0.5,
    gap_min=1,
    gap_max=10,
    gap_noise=0.0
):
    """
    Returns:
        {
            "S": [S^(1), ..., S^(m)],
            "G": [G^(1), ..., G^(m)],
            "I": [I^(1), ..., I^(m)],
            "S0": base sequence
        }
    """

    # --- base (ancestor) sequence ---
    S0 = generate_sequence(n, entropy)

    S_list = []
    I_list = []
    G_list = []

    for _ in range(m):
        # generate sequence via mutation
        Sj = mutate(S0, mutation_rate)

        # compute influence
        Ij = compute_influence(Sj, k, lambda_decay)

        # map to gaps
        Gj = influence_to_gap(Ij, gap_min, gap_max, gap_noise)

        S_list.append(Sj)
        I_list.append(Ij)
        G_list.append(Gj)

    return {
        "S": S_list,
        "G": G_list,
        "I": I_list,
        "S0": S0
    }
 

P = generate_instance_m()
print(P)
 
#“We generate gap constraints via an inverse mapping of local thermodynamic stability derived from nearest-neighbor interactions, resulting in structure-aware benchmark instances with controllable heterogeneity.”



