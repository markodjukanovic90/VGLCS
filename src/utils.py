##ukljucivanje podataka:
from typing import List, Tuple, Sequence, Optional

def load_instance(filepath):
    with open(filepath, 'r') as f:
        lines = [line.strip() for line in f if line.strip() != '']
    
    num_seqs = int(lines[0])
    assert num_seqs == 2, "Trenutno se podržava samo slučaj sa dvije sekvence"

    A = lines[1]
    GA = list(map(int, lines[2].split()))

    B = lines[3]
    GB = list(map(int, lines[4].split()))

    assert len(A) == len(GA), "Dužina A i GA se ne poklapa"
    assert len(B) == len(GB), "Dužina B i GB se ne poklapa"

    return A, B, GA, GB

def load_n_instances(filepath: str) -> Tuple[List[str], List[List[int]]]:
    """
        N                       
        S₁                      
        g₁₀ g₁₁ … g₁(|S₁|-1)     
        S₂
        g₂₀ g₂₁ …
        …
        Sᴺ
        gᴺ₀ gᴺ₁ …
    """
    with open(filepath, 'r') as f:
        # Prazne linije
        lines = [line.strip() for line in f if line.strip()]

    # Header
    try:
        num_seqs = int(lines[0])
    except (IndexError, ValueError):
        raise ValueError("Prva neprazna linija mora sadrzati broj sekvenci (integer ≥ 2)")
    if num_seqs < 2:
        raise ValueError("Instanca mora sadrzati bar 2 sekvence (N ≥ 2)")

    # Sekvence/Gapovi
    seqs: List[str] = []
    gaps: List[List[int]] = []
    cursor = 1
    for i in range(num_seqs):
        try:
            seq = lines[cursor]
            gap_line = lines[cursor + 1]
        except IndexError:
            raise ValueError(f"Fajl prerano zavrsio tokom citanja sekvence #{i + 1} i njenih razmaka")

        gap_vals = list(map(int, gap_line.split()))
        if len(seq) != len(gap_vals):
            raise ValueError(
                f"Ne poklapaju se duzine sekvence #{i + 1}: |S| = {len(seq)}, |G| = {len(gap_vals)}"
            )

        seqs.append(seq)
        gaps.append(gap_vals)
        cursor += 2

    return seqs, gaps

def save_in_file(path, info):

    with open(path, 'w') as f:
        f.write(info)


def reconstruct_from_basic_vglcs_dp(A, B, G_A, G_B, V, F):
    n, m = len(A), len(B)
    max_len = 0
    end_i = end_j = -1

    # Find the position with the maximum value in F
    for i in range(1, n + 1):
        for j in range(1, m + 1):
            if F[i][j] > max_len:
                max_len = F[i][j]
                end_i, end_j = i, j

    sequence = []
    positions = []

    i, j = end_i, end_j

    while F[i][j] > 0:
    
        sequence.append(A[i - 1])
        positions.append((i - 1, j - 1))  # 0-based positions in strings

        found = False
        for i_ in range(1, i):
            if i - i_ <= G_A[i - 1] + 1:
                for j_ in range(1, j):
                    if j - j_ <= G_B[j - 1] + 1 and A[i_-1] == B[j_-1]:
                        if F[i_][j_] == F[i][j] - 1:
                            i, j = i_, j_
                            found = True
                            break
                if found:
                    break
        if not found:
            break

    return ''.join(reversed(sequence)), list(reversed(positions))

    
'''
checking feasibility of solution @s: TODO
'''
def check_feasibility(s, A, B, G_A, G_B):
    
    
    for i in range(1, len(s)):
        #i-1 and i are compared ...
        diff_x =  s[i][0] - s[i-1][0]
        diff_y =  s[i][1] - s[i-1][1]
        
        if diff_x > G_A[ s[i][0] ] + 1 or diff_y > G_B[ s[i][1] ] + 1:  
            print(diff_x,  " ", diff_y, " should be: " ,  G_A[ s[i][0] ] + 1, " and " ,  G_B[ s[i][1] ] + 1 )
            return False
          
    return True  
    

def check_feasibility_n(trace: List[Tuple[int, ...]],
                        gaps: Sequence[Sequence[int]],
                        seqs: Optional[Sequence[str]] = None,
                        verbose: bool = False) -> bool:
    if len(trace) <= 1:
        return True

    N = len(gaps)
    for step in range(1, len(trace)):
        prev, cur = trace[step - 1], trace[step]
        for k in range(N):
            diff = cur[k] - prev[k]
            limit = gaps[k][cur[k]] + 1
            if diff > limit:
                if verbose:
                    name = f"seq#{k+1}"
                    if seqs:
                        name += f" ('{seqs[k][cur[k]]}')"
                    print(f"Jump too large in {name}: {diff} > {limit} (from {prev[k]} -> {cur[k]})")
                return False
    return True
   
    
