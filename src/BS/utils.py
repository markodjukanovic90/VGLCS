# TODO: build_next_occurrence

from collections import defaultdict

def build_next_occurrence(sequences, alphabet):
    """
    Build a 3D dictionary: next_occurrence[i][j][a] = next position of 'a' in s_i at or after j (1-based).
    
    Args:
        sequences: list of strings or character lists (indexed from 0, but we use 1-based logic)
        alphabet: set of all possible characters Σ

    Returns:
        next_occurrence: list of 2D dicts for each sequence s_i
    """
    next_occurrence = []

    for s in sequences:
        n = len(s)
        # Initialize next_pos[a] = n+1 for all a in Σ (means "not found")
        next_pos = {a: n + 1 for a in alphabet}
        occ_table = [dict() for _ in range(n + 2)]  # 1-based indexing, plus extra for position n+1

        # Traverse sequence in reverse to fill next_pos map at each position
        for i in range(n, 0, -1):  # from n to 1
            next_pos[s[i - 1]] = i  # Update next position of current symbol ==> FIX??
            occ_table[i] = next_pos.copy()

        # Add dummy last row (position n+1) — no further occurrences
        occ_table[n + 1] = {a: n + 1 for a in alphabet}
        next_occurrence.append(occ_table)

    return next_occurrence
    

def extend(v, sequences, G, next_occurrence):
    """
    Generate all non-dominated extensions (children) of node v.

    Args:
        v: A tuple (positions, length), where:
            positions is a list [p1, ..., pm] of current indices in each sequence (1-based),
            length is the current length of the partial solution.
        sequences: A list of sequences [s1, s2, ..., sm].
        G: A list of gap functions [G1, ..., Gm], each Gi: int -> int.
        next_occurrence: A list of dictionaries next_occurrence[i][a] = list of next positions of a in sequence i (1-based).

    Returns:
        List of children nodes, each in form ([new_p1, ..., new_pm], length+1).
    """
    m = len(sequences)
    positions, l_v = v
    candidates = {}  # Map from symbol a to its first positions [p1^a, ..., pm^a]

    # Step 1: Determine all valid symbols a ∈ Σ^v that can be matched in all sequences
    for a in set.intersection(*[
        set([c for idx, c in enumerate(seq[p-1:], start=p) 
             if idx - p <= G[i](idx)]) 
        for i, (seq, p) in enumerate(zip(sequences, positions))
    ]):
        pos_a = []
        valid = True
        for i in range(m):
            # Get next occurrence ≥ positions[i] of a in sequence i
            occ_list = next_occurrence[i].get(a, [])
            pos = next((idx for idx in occ_list if idx >= positions[i]), -1)
            if pos == -1 or pos - positions[i] > G[i](pos):
                valid = False
                break
            pos_a.append(pos)
        if valid:
            candidates[a] = pos_a

    # Step 2: Dominance filtering over candidates
    non_dominated = []
    for a, pos_a in candidates.items():
        dominated = False
        for b, pos_b in candidates.items():
            if a == b:
                continue
            if all(pa <= pb for pa, pb in zip(pos_a, pos_b)):
                if all(G[i](pos_b[i]) + pos_a[i] + 1 >= pos_b[i] for i in range(m)):
                    dominated = True
                    break
        if not dominated:
            non_dominated.append((a, pos_a))

    # Step 3: Create child nodes from non-dominated extensions
    children = []
    for a, pos_a in non_dominated:
        new_positions = [pi + 1 for pi in pos_a]
        children.append((new_positions, l_v + 1))

    return children

def heuristic(node):
    """
    Jedna od heuristika, npr. dužina sekvence (h1).
    """
    return node.length  # h1
