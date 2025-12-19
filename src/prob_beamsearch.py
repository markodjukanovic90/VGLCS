from __future__ import annotations
from dataclasses import dataclass, field
from typing import List, Tuple, Dict, Optional
from utils import check_feasibility_n, load_n_instances
import collections
import random
import math

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


def probabilistic_beam_select(
    candidates: List["Node"],
    sequences: List[str],
    gaps: List[List[int]],
    beam_width: int,
    heuristic: str,
    alpha: float,
    beta: float,
    lam: float,
    k: int,
    temperature: float = 1.0
) -> List["Node"]:
    """
    Odabir čvorova za beam search koristeći vjerovatnosni pristup.
    Parametri
    ----------
    temperature : float
        Lower = greedier, Higher = more exploratory (softmax temperature)
    """
    # Heuristčki skoring
    scored = [
        (n, score(n, sequences, gaps, heuristic=heuristic, alpha=alpha, beta=beta, lam=lam, k=k))
        for n in candidates
    ]

    # Softmax
    max_score = max(s for _, s in scored)
    weights = [math.exp((s - max_score) / temperature) for _, s in scored]
    total = sum(weights)
    probs = [w / total for w in weights]

    # Uzorkovanje bez vraćanja
    selected = random.choices(
        population=[n for n, _ in scored],
        weights=probs,
        k=min(beam_width, len(candidates))
    )
    return selected


#  Generisanje novih čvorova

def _first_valid_indices(seq: str, start_idx: int, gap_arr: List[int], prev_idx: int) -> Dict[str, int]:
    """Vraća za *jednu* sekvencu dict char -> najmanji indeks >= start_idx koji 
    zadovoljava gap constraint u odnosu na prev_idx (-1 znači prvi karakter).
    """
    mapping: Dict[str, int] = {}
    for j in range(start_idx, len(seq)):
        if prev_idx != -1 and (j - prev_idx) > gap_arr[j] + 1:
            # Prevelik razmak kada je *trenutni* kandidat u pitanju
            continue
        c = seq[j]
        if c not in mapping:
            # Samo prvi (najdalje lijevo) hit per char.
            mapping[c] = j
    return mapping


def generate_successors(node: Node, sequences: List[str], gaps: List[List[int]]) -> List[Node]:
    """Vraća nasljednike čvora node koji poštuju gap pravila"""
    m = len(sequences)

    per_seq_maps = [
        _first_valid_indices(sequences[i], node.pos[i] + 1, gaps[i], node.pos[i])
        for i in range(m)
    ]

    # Karakteri koja svaka sekvenca može mečovati
    common_chars = set.intersection(*(set(d.keys()) for d in per_seq_maps))
    if not common_chars:
        return []

    # Uklanjanje dominirajućih simbola

    dom = set()
    for c in common_chars:
        for o in common_chars:
            if c == o:
                continue
            if all(d[c] <= d[o] and gaps[i][d[o]] + d[c] + 1 >= d[o] for i, d in enumerate(per_seq_maps)):
                dom.add(o)
    common_chars.difference_update(dom)

    # Jedan kandidat (indeks vektor) po zajedničkom karakteru
    draft: List[Node] = []
    for ch in sorted(common_chars):
        # minimalni indeksi
        idx_vector = tuple(d[ch] for d in per_seq_maps)  
        jumps      = tuple(0 if node.pos[i] == -1 else idx_vector[i] - node.pos[i] for i in range(m))
        draft.append(Node(idx_vector, node.seq + ch, jumps, node))

    return draft


#Heuristike TODO - mozda refaktorisati da se izbjegnu if-ovi, ok za sada

def _remaining_lb(node: Node, sequences: List[str]) -> int:
    return min(len(sequences[i]) - node.pos[i] - 1 for i in range(len(sequences)))


def _gap_score(node: Node, gaps: List[List[int]]) -> int:
    vals = [gaps[i][node.pos[i]] if node.pos[i] != -1 else max(gaps[i])
            for i in range(len(gaps))]
    return min(vals)


def _lookahead_k(node: Node, sequences: List[str], k: int = 5) -> int:
    ahead_sets = []
    for i, seq in enumerate(sequences):
        start = node.pos[i] + 1
        ahead_sets.append(set(seq[start:start + k]))
    return len(set.intersection(*ahead_sets)) if ahead_sets else 0


def _freq_alignment(node: Node, sequences: List[str]) -> int:
    remains = [collections.Counter(seq[node.pos[i] + 1:]) for i, seq in enumerate(sequences)]
    alphabet = set().union(*(c.keys() for c in remains))
    return sum(min(c[ch] for c in remains) for ch in alphabet)


def _jump_penalty(node: Node, lam: float) -> float:
    return lam * sum(node.parent_jump)


def score(node: Node, sequences: List[str], gaps: List[List[int]], *, heuristic: str,
          alpha: float = 1.0, beta: float = 1.0, lam: float = 1.0, k: int = 5) -> float:
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
        return l + _freq_alignment(node, sequences)
    if heuristic == "h6":
        return l - _jump_penalty(node, lam)
    if heuristic == "h7":
        return alpha * l + beta * _remaining_lb(node, sequences) - _jump_penalty(node, lam)
    raise ValueError(f"Unknown heuristic: {heuristic}")


#Beam Search (Algoritam 4)

def prob_beam_search_glcs(
    sequences: List[str],
    gaps: List[List[int]],
    *,
    beam_width: int = 10,
    heuristic: str = "h1",
    alpha: float = 1.0,
    beta: float = 1.0,
    lam: float = 1.0,
    k: int = 5,
    max_iters: int = 10000,
    max_resets: int = 100
) -> Tuple[str, List[Tuple[int, ...]]]:
    """
    Parametri
    ---------
    sequences : list[str]
        M ulaznih sekvenci.
    gaps : list[list[int]]
        Niz razmaka za svaku sekvencu.
    beam_width : int, default 10
        Širina bima.
    heuristic : {"h1",…, "h7"}, default "h1"
        Koja heuristika se koristi za rangiranje.
    alpha, beta, lam : float
        Težine za h6/h7.
    k : int, default 5
        Veličina prozora za h4.
    max_iters : int, default 10 000
        Ograničenje broja proširivanja.
    max_resets : int, default 100
        Broj resetujućih iteracija.
    """
    m = len(sequences)
    if not (len(gaps) == m and all(len(g) == len(sequences[i]) for i, g in enumerate(gaps))):
        raise ValueError("Svaka sekvenca mora imati niz razmaka jednake dužine.")
    
    best_result = ""
    steps_result = []
    for iter in range(max_resets):
        # Početno stanje, -1 znači da još nije odabran karakter
        beam: List[Node] = [Node(tuple([-1] * m), "", tuple([0] * m))]
        best = ""
        return_node = None

        for _ in range(max_iters):
            if not beam:
                break
            candidates: List[Node] = []

            # Proširivanje svakog čvora u bimu
            for node in beam:
                for succ in generate_successors(node, sequences, gaps):
                    candidates.append(succ)
                    if len(succ.seq) > len(best):
                        best = succ.seq
                        return_node = succ
            
            # Bim prazan
            if not candidates:
                break

            # Odabir čvorova za sljedeći korak
            beam = probabilistic_beam_select(
                candidates,
                sequences,
                gaps,
                beam_width,
                heuristic,
                alpha,
                beta,
                lam,
                k,
                temperature=1.0   #Probati 0.5 za greedy, 2.0 za random
            )

        steps = []
        if return_node is not None:
            best = return_node.seq
            while return_node.parent is not None:
                steps.append(return_node.pos)
                return_node = return_node.parent
            steps.reverse()
        if len(best) > len(best_result):
            best_result = best
            steps_result = steps
    return best_result, steps_result


if __name__ == "__main__":
    instances = load_n_instances("instance_i_generator/instance/sigma-2/mglcs_2_200_2_8.txt")
    sequences = instances[0]
    gaps = instances[1]

    for h in ("h1", "h2", "h3", "h4", "h5", "h6", "h7"):
        res = prob_beam_search_glcs(sequences, gaps, beam_width=10, heuristic=h)
        print(check_feasibility_n(res[1], gaps, sequences))
        print(res)
        print(len(res[0]))
