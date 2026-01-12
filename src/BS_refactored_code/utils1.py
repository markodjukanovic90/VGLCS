from typing import List, Tuple, Sequence, Optional
import collections

def load_n_instances(filepath: str) -> Tuple[List[str], List[List[int]]]:
    with open(filepath, 'r') as f:
        lines = [line.strip() for line in f if line.strip()]
    num_seqs = int(lines[0])
    seqs: List[str] = []
    gaps: List[List[int]] = []
    cursor = 1
    for i in range(num_seqs):
        seq = lines[cursor]
        gap_line = lines[cursor + 1]
        gap_vals = list(map(int, gap_line.split()))
        if len(seq) != len(gap_vals):
            raise ValueError(f"Sequence {i+1} and gaps length mismatch")
        seqs.append(seq)
        gaps.append(gap_vals)
        cursor += 2
    return seqs, gaps

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
                    print(f"Jump too large in {name}: {diff} > {limit}")
                return False
    return True

