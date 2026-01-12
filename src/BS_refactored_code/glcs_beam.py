import argparse
import collections
import time
from dataclasses import dataclass, field
from typing import List, Tuple, Optional, Set, Dict
from utils1 import check_feasibility_n, load_n_instances

import heapq
import argparse


@dataclass
class Node:
    """Stanje u grafu."""
    pos: Tuple[int, ...]            # Indeksi zadnjeg meča za svaku sekvencu (-1 znači prije početka)
    seq: str                        # Trenutno izgrađeni LCS
    parent_jump: Tuple[int, ...] = field(default_factory=tuple)  # Skokovi po sekvenci kako bi se doslo do ovog cvora
    parent: Optional[Node] = field(
        default=None,
        repr=False,
        compare=False
    )
    @property
    def length(self) -> int:
        return len(self.seq)



def build_prev_table(sequences: List[str], Sigma: List[str]) -> List[Dict[str, List[int]]]:
    """
    Prev[i][a][j] = max p < j s.t. sequences[i][p] == a, else -1.
    """
    Prev: List[Dict[str, List[int]]] = []
    for s in sequences:
        n = len(s)
        prev_i = {a: [-1] * (n + 1) for a in Sigma}
        last = {a: -1 for a in Sigma}
        for j in range(n + 1):
            for a in Sigma:
                prev_i[a][j] = last[a]
            if j < n:
                last[s[j]] = j
        Prev.append(prev_i)
    return Prev


def build_suffix_counts(sequences: List[str], Sigma: List[str]) -> List[Dict[str, List[int]]]:
    """
    C[i][a][j] = frekvencija chara 'a' u sekvencama[i][j:]
    Nizovi imaju dužinu len(s_i)+1, sa zadnjom ćelijom = 0 (prazan sufiks).
    Time/space: O(sum_i |s_i| * |Sigma|)
    """
    C: List[Dict[str, List[int]]] = []
    for s in sequences:
        n = len(s)
        Ci = {a: [0]*(n+1) for a in Sigma}
        for j in range(n-1, -1, -1):
            ch = s[j]
            for a in Sigma:
                Ci[a][j] = Ci[a][j+1]
            Ci[ch][j] += 1
        C.append(Ci)
    return C

def score(node: Node, sequences: List[str], gaps: List[List[int]], *, heuristic: str,
          alpha: float = 1.0, beta: float = 1.0, lam: float = 1.0, k: int = 5, kh8: int = None,
          sigma: int = None, P_table: List[List[float]] = None, gap_suffixes: List[int] = None,
          C_h5: List[Dict[str, List[int]]] = None, Sigma_h5: List[str] = None, seq_lens: List[int] = None) -> float:
    l = node.length
    if heuristic == "h1":
        return l
    if heuristic == "h2":
        return l + _remaining_lb(node, sequences)
    if heuristic == "h3":
        return _gap_score(node, gaps)
    if heuristic == "h4":
        return l + _lookahead_k(node, sequences, k)
    if heuristic == "h5":
        return l + _h5_upper_bound(node.pos, C_h5, Sigma_h5, seq_lens)
    if heuristic == "h6":
        return l - _jump_penalty(node, lam)
    if heuristic == "h7":
        return alpha * l + beta * _remaining_lb(node, sequences) - _jump_penalty(node, lam)
    if heuristic == "h8":
        # preostale dužine
        rem = [len(sequences[i]) - node.pos[i] for i in range(len(sequences))]
        score = 1.0
        for i, r in enumerate(rem):
            # P(rem[i], k)
            p_val = P_table[r][kh8] if r >= 0 else 0.0
            # gap indikator
            score *= p_val
        return score
    raise ValueError(f"Unknown heuristic: {heuristic}")WW


def _mk_root_node(pos: Tuple[int, ...], m: int) -> Node:
    assert len(pos) == m
    return Node(pos=pos, seq="", parent=None)




#==============================================================
### Backward BS approach (bi-directional BS)
#=============================================================

def generate_backward_successors(

    node: Node,
    sequences: List[str],
    gaps: List[List[int]],
    Prev,
    Sigma,
) -> List[Node]:

    m = len(sequences)
    p = node.pos # p-vector of positions from where to start the backward pass
    
    successors: List[Node] = []

    for a in Sigma:
        prev_positions = []

        feasible = True
        for i in range(m):
            pi = p[i]
            if pi < 0:
                feasible = False
                break
            pa = Prev[i][pi][a]
            
            if pa == -1:
                feasible = False
                break
            if pi - pa > gaps[i][pi] + 1
                feasible = False
                break 
            
            prev_positions.append(pa)

        if not feasible:
            continue

        # build child
        new_pos = tuple(pa - 1 for pa in prev_positions)
        child = Node(
            pos=new_pos,
            seq=node.seq + a,   # backward → append
            parent=node,
        )
        successors.append(child)

    return successors


def beam_search_glcs_backward(
    sequences: List[str],
    gaps: List[List[int]],
    start_pos: Tuple[int, ...],
    Prev,
    Sigma,
    *,
    beam_width: int = 10,
    heuristic: str = "h1",
    alpha: float = 1.0,
    beta: float = 1.0,
    lam: float = 1.0,
    k: int = 5,
    max_iters: int = 10_000,
) -> Tuple[str, List[Tuple[int, ...]]]:
    """
    Backward Beam Search for Variable-Gapped LCS.

    Starts from position vector p and expands backward.
    """

    m = len(sequences)

    root = Node(start_pos, "", None)
    beam: List[Node] = [root]

    best_node: Optional[Node] = None
    best_len = 0

    score_args = {
        "heuristic": heuristic,
        "alpha": alpha,
        "beta": beta,
        "lam": lam,
        "k": k,
    }

    for _ in range(max_iters):
        if not beam:
            break

        candidates: List[Node] = []

        for node in beam:
            succs = generate_backward_successors(
                node, sequences, gaps, Prev, Sigma
            )

            for succ in succs:
                candidates.append(succ)
                if succ.length > best_len:
                    best_len = succ.length
                    best_node = succ

        if not candidates:
            break

        beam = sorted(
            candidates,
            key=lambda n: score(n, sequences, gaps, **score_args),
            reverse=True,
        )[:beam_width]

    # ------------------------------------------------------------
    # Reconstruct backward solution
    # ------------------------------------------------------------

    steps: List[Tuple[int, ...]] = []
    seq = ""

    if best_node is not None:
        node = best_node
        seq = node.seq[::-1]  # reverse backward-built sequence

        while node.parent is not None:
            steps.append(node.pos)
            node = node.parent
        steps.reverse()

    return seq, steps #for the start_pos we return (seq, steps) pair ... 



# ============================================================
# IMBS–GLCS main algorithm
# ============================================================

def imbs_glcs(
    sequences: List[str],
    gaps: List[List[int]],
    *,
    beam_width: int = 10,
    heuristic: str = "h5",
    imbs_iters: int = 50,
    number_of_roots: int = 1,
    alpha: float = 1.0,
    beta: float = 1.0,
    lam: float = 1.0,
    k: int = 5,
    max_iters: int = 10000,
) -> Tuple[str, List[Tuple[int, ...]]]:
    """
    Iterative Multi-Source Beam Search (IMBS) for GLCS.
    """

    m = len(sequences)
    if not (len(gaps) == m and all(len(g) == len(sequences[i]) for i, g in enumerate(gaps))):
        raise ValueError("Each sequence must have a gap list of equal length.")

    # ------------------------------------------------------------
    # Heuristic preparation
    # ------------------------------------------------------------

    score_args: Dict = {
        "heuristic": heuristic,
        "alpha": alpha,
        "beta": beta,
        "lam": lam,
        "k": k,
    }

    Sigma = sorted(set().union(*sequences))
    Prev = build_prev_table(sequences, Sigma)
    seq_lens = [len(s) for s in sequences]

    if heuristic == "h5":
        C_h5 = build_suffix_counts(sequences, Sigma)
        score_args.update({
            "C_h5": C_h5,
            "Sigma_h5": Sigma,
            "seq_lens": seq_lens,
        })

    elif heuristic == "h8":
        sigma = len(Sigma)
        max_len = max(seq_lens)
        score_args.update({
            "sigma": sigma,
            "P_table": build_P_table(max_len, sigma),
        })

    # ------------------------------------------------------------
    # Global best
    # ------------------------------------------------------------

    best_seq: str = ""
    best_steps: List[Tuple[int, ...]] = []

    # ------------------------------------------------------------
    # Root set R
    # ------------------------------------------------------------

    global_root = tuple([-1] * m)
    R: List[Tuple[int, ...]] = [global_root]
    R_seen: Set[Tuple[int, ...]] = {global_root}

    # ============================================================
    # Local Beam Search from given sources
    # ============================================================

    def run_local_beam(
        sources: List[Tuple[int, ...]]
    ) -> Tuple[str, List[Tuple[int, ...]], Optional[Tuple[int, ...]], List[Tuple[int, ...]]]:

        beam: List[Node] = [_mk_root_node(src, m) for src in sources]
        harvested_roots: Set[Tuple[int, ...]] = set()

        best_local = ""
        best_node: Optional[Node] = None

        iters = 0
        while beam and iters < max_iters:
            iters += 1
            candidates: List[Node] = []

            for node in beam:
                successors = generate_successors(node, sequences, gaps)

                # --- dead-end: generate new roots
                if not successors:
                    for rpos in _new_roots_from_node(node, sequences, gaps):
                        if rpos in R_seen:
                            continue
                        if not _feasible_root(rpos, sequences, len(best_seq)):
                            continue
                        if not is_true_root_like(rpos, gaps, Prev, Sigma):
                            continue
                        harvested_roots.add(rpos)
                    continue

                # --- normal expansion
                for succ in successors:
                    candidates.append(succ)
                    if succ.length > len(best_local):
                        best_local = succ.seq
                        best_node = succ

            if not candidates:
                break

            # --- heuristic-dependent updates
            if heuristic == "h8":
                rems = [
                    min(len(sequences[i]) - v.pos[i] + 1 for i in range(m))
                    for v in candidates
                ]
                score_args["kh8"] = max(1, min(rems) // score_args["sigma"])

            # --- beam truncation
            beam = sorted(
                candidates,
                key=lambda n: score(n, sequences, gaps, **score_args),
                reverse=True
            )[:beam_width]

        # --------------------------------------------------------
        # Extract solution path
        # --------------------------------------------------------

        steps: List[Tuple[int, ...]] = []
        last_pos = None

        if best_node is not None:
            last_pos = best_node.pos
            node = best_node
            while node.parent is not None:
                steps.append(node.pos)
                node = node.parent
            steps.reverse()

        return best_local, steps, last_pos, list(harvested_roots)

    # ============================================================
    # IMBS outer loop
    # ============================================================

    for _ in range(imbs_iters):
        if not R:
            break

        # 1) rank roots
        root_nodes = [_mk_root_node(pos, m) for pos in R]
        root_nodes.sort(
            key=lambda n: score(
                n,
                sequences,
                gaps,
                heuristic="h5",
                C_h5=score_args.get("C_h5"),
                Sigma_h5=score_args.get("Sigma_h5"),
                seq_lens=seq_lens,
            ),
            reverse=True,
        )

        R = [n.pos for n in root_nodes]
        sources = R[:number_of_roots]
        R = R[number_of_roots:]

        # 2) local beam search
        seq_local, steps_local, _, new_roots = run_local_beam(sources)

        # 3) update global best
        if len(seq_local) > len(best_seq):
            best_seq = seq_local
            best_steps = steps_local

        # 4) add new roots
        for pos in new_roots:
            if pos not in R_seen and _feasible_root(pos, sequences, len(best_seq)):
                R.append(pos)
                R_seen.add(pos)

    return best_seq, best_steps
    

def parse_args():
    parser = argparse.ArgumentParser(
        description="Iterative Multi-Source Beam Search for GLCS/VGLCS"
    )

    # input
    parser.add_argument("--input", type=str, required=True,
                        help="Path to input instance file")

    # algorithm choice
    parser.add_argument("--algorithm", type=str, default="imbs",
                        choices=["imbs", "bs"],
                        help="Algorithm to run")

    # beam / IMBS parameters
    parser.add_argument("--beam-width", type=int, default=100)
    parser.add_argument("--heuristic", type=str, default="h8")
    parser.add_argument("--imbs-iters", type=int, default=10)
    parser.add_argument("--number-of-roots", type=int, default=10)

    # heuristic parameters
    parser.add_argument("--alpha", type=float, default=1.0)
    parser.add_argument("--beta", type=float, default=1.0)
    parser.add_argument("--lam", type=float, default=1.0)
    parser.add_argument("--k", type=int, default=5)
    
    #out file path 
    parser.add_argument("--output", type=str, default=None,    help="Output file to write run statistics")

    # stopping
    parser.add_argument("--max-iters", type=int, default=10000)

    return parser.parse_args()


def format_stats(args, runtime, feasible, final_seq, final_steps):
    lines = []
    lines.append("=" * 60)
    lines.append(f"Algorithm     : {args.algorithm.upper()}")
    lines.append(f"Heuristic     : {args.heuristic}")
    lines.append(f"Beam width    : {args.beam_width}")

    if args.algorithm == "imbs":
        lines.append(f"IMBS iters    : {args.imbs_iters}")
        lines.append(f"#roots        : {args.number_of_roots}")

    lines.append(f"Alpha         : {args.alpha}")
    lines.append(f"Beta          : {args.beta}")
    lines.append(f"Lambda        : {args.lam}")
    lines.append(f"k             : {args.k}")
    lines.append("-" * 60)
    lines.append(f"Time [s]      : {runtime:.6f}")
    lines.append(f"Feasible      : {'YES' if feasible else 'NO'}")
    lines.append(f"LCS length    : {len(final_seq)}")
    lines.append(f"LCS           : {final_seq}")
    lines.append(f"Steps         : {final_steps}")
    lines.append("=" * 60)

    return "\n".join(lines)
    
    

def main():
    args = parse_args()

    sequences, gaps = load_n_instances(args.input)
    final_seq, final_steps = "", []  # initialize to avoid UnboundLocalError
    

    import time
    start = time.time()

    if args.algorithm == "imbs":
        final_seq, final_steps = imbs_glcs(
            sequences,
            gaps,
            beam_width=args.beam_width,
            heuristic=args.heuristic,
            imbs_iters=args.imbs_iters,
            number_of_roots=args.number_of_roots,
            alpha=args.alpha,
            beta=args.beta,
            lam=args.lam,
            k=args.k,
            max_iters=args.max_iters
        )

    elif args.algorithm == "bs":
        final_seq, final_steps = beam_search_glcs(
            sequences,
            gaps,
            beam_width=args.beam_width,
            heuristic=args.heuristic,
            alpha=args.alpha,
            beta=args.beta,
            lam=args.lam,
            k=args.k,
            max_iters=args.max_iters
        )

    end = time.time()
    runtime = end - start

    # Ensure final_steps is never None for feasibility check
    if final_steps is None:
        final_steps = []

    feasible = check_feasibility_n(final_steps, gaps, sequences)

    stats = format_stats(
        args,
        runtime,
        feasible,
        final_seq,
        final_steps
    )

    if args.output is not None:
        with open(args.output, "w") as f:
            f.write(stats + "\n")

    # Also print to console
    print(stats)


if __name__ == "__main__":
    main()

