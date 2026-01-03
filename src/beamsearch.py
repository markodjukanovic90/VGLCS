from __future__ import annotations
from dataclasses import dataclass, field
from typing import List, Tuple, Dict, Set, Iterable, Optional
from utils1 import check_feasibility_n, load_n_instances
import collections
import heapq
import argparse
import time 

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

def _h5_upper_bound(node_pos: Tuple[int, ...],
                    C_h5: List[Dict[str, List[int]]],
                    Sigma_h5: List[str],
                    seq_lens: List[int]) -> int:
    """
    UB(v) = sum_{σ in Σ} min_i C[i][ σ ][ p_i^v ],
    gdje je p_i^v = node_pos[i] + 1 (sljedeći indeks nakon zadnjeg meča).
    """
    m = len(C_h5)
    j_starts = [min(node_pos[i] + 1, seq_lens[i]) for i in range(m)]
    ub = 0  
    for a in Sigma_h5:
        mn = None
        for i in range(m):
            cnt = C_h5[i][a][j_starts[i]]
            mn = cnt if mn is None else (cnt if cnt < mn else mn)
        ub += 0 if mn is None else mn
    return ub


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

def build_P_table(max_n: int, sigma: int) -> List[List[float]]:
    """P[n][k] za 0<=n,k<=max_n. (jednakost (9))"""
    P = [[0.0]*(max_n+1) for _ in range(max_n+1)]
    for i in range(max_n+1):
        P[i][0] = 1.0
    # P[0][j>0] ostaje 0
    for n in range(1, max_n+1):
        for k in range(1, max_n+1):  
            P[n][k] = (P[n-1][k-1] + (sigma-1)*P[n-1][k]) / sigma
    return P


def h5_backward(node: Node,
                    C_h5: List[Dict[str, List[int]]],
                    Sigma_h5: List[str],
                    seq_lens: List[int]) -> int:
    """
    Backward h5 heuristic:
    estimates how many symbols can still be prepended
    from prefixes s_i[:p_i].
    """

    m = len(node.pos)
    h = 0

    for a in Sigma_h5:
        min_count = float("inf")

        for i in range(m):
            p_i = node.pos[i]

            if p_i < 0:
                min_count = 0
                break

            # prefix freq = total freq - suffix freq
            prefix_count = C_h5[i][a][0] - C_h5[i][a][p_i + 1]
            min_count = min(min_count, prefix_count)

        h += min_count

    return h



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
    raise ValueError(f"Unknown heuristic: {heuristic}")
  

#Beam Search (Algoritam 4)

def beam_search_glcs(
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
    time_limit: int = 1800
) -> Tuple[str, List[Tuple[int, ...]], Optional[Tuple[int, ...]]]:
    """
    Parametri
    ---------
    sequences : list[str]
        M ulaznih sekvenci.
    gaps : list[list[int]]
        Niz razmaka za svaku sekvencu.
    beam_width : int, default 10
        Širina bima.
    heuristic : {"h1",…, "h8"}, default "h1"
        Koja heuristika se koristi za rangiranje.
    alpha, beta, lam : float
        Težine za h6/h7.
    k : int, default 5
        Veličina prozora za h4.
    max_iters : int, default 10 000
        Ograničenje broja proširivanja.
    """
    m = len(sequences)
    if not (len(gaps) == m and all(len(g) == len(sequences[i]) for i, g in enumerate(gaps))):
        raise ValueError("Svaka sekvenca mora imati niz razmaka jednake dužine.")

    # Početno stanje, -1 znači da još nije odabran karakter
    beam: List[Node] = [Node(tuple([-1] * m), "", tuple([0] * m))]
    best = ""
    return_node = None
    score_args = {
        'heuristic' : heuristic,
        'alpha' : alpha,
        'beta' : beta,
        'lam' : lam,
        'k' : k
    }

    if heuristic == "h8":
        sigma = len(set().union(*sequences))
        max_len = max(len(s) for s in sequences)
        score_args['sigma'] = sigma
        score_args['P_table'] = build_P_table(max_len, sigma)
    elif heuristic == "h5":
        Sigma_h5 = sorted(set().union(*sequences))
        C_h5 = build_suffix_counts(sequences, Sigma_h5)
        seq_lens = [len(s) for s in sequences]
        score_args['C_h5'] = C_h5
        score_args['Sigma_h5'] = Sigma_h5
        score_args['seq_lens'] = seq_lens

    
    time_start = time.time()
    
    for _ in range(max_iters):
        if not beam:
            break
        # Provera vremenskog ograničenja
        if time.time() - time_start > time_limit:
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

        # k za h8 (13)
        if heuristic == "h8": 
            # za svako v iz kandidata: min_i (|s_i| - p_i^v + 1)
            rems_per_v = [
                min(len(sequences[i]) - v.pos[i] + 1 for i in range(m))
                for v in candidates
            ]
            min_rem = min(rems_per_v)
            score_args['kh8'] = max(1, min_rem // sigma)

        # Rangiranje i skraćivanje bima
        ranked = sorted(
            candidates,
            key=lambda n: score(n, sequences, gaps, **score_args),
            reverse=True,
        )
        beam = ranked[:beam_width]

    steps = []
    last_pos = return_node.pos if return_node is not None else None
    if return_node is not None:
        best = return_node.seq
        while return_node.parent is not None:
            steps.append(return_node.pos)
            return_node = return_node.parent
        steps.reverse()

    return best, steps, time.time() - time_start

def _first_indices_no_gap(seq: str, start_idx: int) -> Dict[str, int]:
    """
    Za jednu sekvencu vraća mapu: karakter -> najmanji indeks >= start_idx,
    BEZ provjere gap ograničenja (p^w).
    """
    mapping: Dict[str, int] = {}
    for j in range(start_idx, len(seq)):
        c = seq[j]
        if c not in mapping:
            mapping[c] = j
    return mapping

def _ub_remaining(node: Node, sequences: List[str]) -> int:
    return min(len(sequences[i]) - (node.pos[i] + 1) for i in range(len(sequences)))


def _feasible_root(last_matched: Tuple[int, ...], sequences: List[str], best_len: int) -> bool:
    """
    'pos' kao indeks POSLJEDNJEG meča (root = -1),
    pa je p^w = last_matched + 1, i uslov je:
        remaining = min_i len(s_i) - (last_matched_i + 1)
        remaining > best_len
    """
    rem = min(len(sequences[i]) - (last_matched[i] + 1) for i in range(len(sequences)))
    return rem > best_len

def _new_roots_from_node(node: Node, sequences: List[str], gaps: List[List[int]]) -> List[Tuple[int, ...]]:
    """
    Za list v: za SVAKO slovo a u presjeku (po svim sekvencama) uzmi minimalne pozicije p^w_i >= pos_v[i]+1
    (bez gap testova), filtriranje za dominaciju, pa vraćanje u formatu 'draft' = p^w - 1.
    """
    m = len(sequences)

    # 1) Mapa karakter->indeks po sekvenci, BEZ gap uslova
    per_seq_maps = [
        _first_indices_no_gap(sequences[i], node.pos[i] + 1)
        for i in range(m)
    ]

    # 2) Zajednička slova kroz sve sekvence
    common_chars = set.intersection(*(set(d.keys()) for d in per_seq_maps)) if per_seq_maps else set()
    if not common_chars:
        return []

    # 3) Uklanjanje dominirajućih simbola
    '''
    dom = set()
    for c in common_chars:
        for o in common_chars:
            if c == o:
                continue
            if all(d[c] <= d[o] and gaps[i][d[o]] + d[c] + 1 >= d[o] for i, d in enumerate(per_seq_maps)):
                dom.add(o)
    common_chars.difference_update(dom)
    '''
    
    #  4) Jedan kandidat (indeks vektor) po zajedničkom karakteru
    draft: List[Tuple[int, ...]] = []
    for ch in sorted(common_chars): 
        # minimalni indeksi
        idx_vector = tuple(d[ ch ] for d in per_seq_maps)  
        draft.append(idx_vector)
    return draft

def _mk_root_node(pos: Tuple[int, ...], m: int) -> Node:
    # Root za IBMS ima prazan seq, i parent_jump=0 za bilježenje.
    return Node(pos=pos, seq="", parent_jump=tuple([0]*m), parent=None)

def _mk_root_node_with_nonempty_seq(pos: Tuple[int, ...], m: int, cache: Dict) -> Node:
    # Root za IBMS ima prazan seq, i parent_jump=0 za bilježenje.
    return Node(pos=pos, seq=cache[pos][0], parent_jump=tuple([0]*m), parent=None)

def build_prev_table(sequences: List[str], Sigma: List[str]) -> List[Dict[str, List[int]]]:
    
    Prev: List[Dict[str, List[int]]] = []

    for s in sequences:
        n = len(s)

        # Prev za jednu sekvencu
        prev_i: Dict[str, List[int]] = {
            a: [-1] * (n + 1) for a in Sigma
        }

        # last[a] = posljednja pozicija znaka a prije trenutnog j
        last = {a: -1 for a in Sigma}

        for j in range(n + 1):
            # u ovom trenutku last[a] = max p < j
            for a in Sigma:
                prev_i[a][j] = last[a]

            if j < n:
                last[s[j]] = j

        Prev.append(prev_i)

    return Prev


def is_true_root_like(pos: Tuple[int, ...],
                      gaps: List[List[int]],
                      Prev: List[Dict[str, List[int]]],
                      Sigma: List[str]) -> bool:
    """
    True  -> zadrži kao root (nema nijednog zajedničkog 'a' u svim gap prozorima).
    False -> odbaci (postoji zajednički 'a' => postoji roditelj => nije root).
    """
    m = len(pos)

    # ako je pos[i] == -1, to je global root; tretiraj kao korijen
    if any(p < 0 for p in pos):
        return True

    # računaj L_i za svaku sekvencu (donja granica prozora)
    L = []
    for i in range(m):
        p = pos[i]
        g = gaps[i][p]           # G_i(p_i)
        L.append(p - g - 1)      # L_i

    # tražimo da postoji *isti* a koji je prisutan u svakom prozoru
    for a in Sigma:
        ok = True
        for i in range(m):
            p = pos[i]
            q = Prev[i][a][p]    # najveći indeks < p gdje je a
            if q < L[i]:
                ok = False
                break
        if ok:
            # našli smo zajednički a u svim prozorima => pos nije korijen
            return False

    return True

def imbs_glcs(
    sequences: List[str],
    gaps: List[List[int]],
    *,
    beam_width: int = 10,
    heuristic: str = "h5",              
    imbs_iters: int = 50,               # max IMBS spoljašnje iteracije
    number_of_roots: int = 1,          # max broj korijena koji se uzimaju iz R
    alpha: float = 1.0,
    beta: float = 1.0,
    lam: float = 1.0,
    k: int = 5,
    max_iters: int = 10000, 
    time_limit: int = 1800        
) -> Tuple[str, List[Tuple[int, ...]]]:
    """
    Iterative Multi-Source Beam Search (Algoritam 6).
    """
    m = len(sequences)
    if not (len(gaps) == m and all(len(g) == len(sequences[i]) for i, g in enumerate(gaps))):
        raise ValueError("Svaka sekvenca mora imati niz razmaka jednake dužine.")

    score_args = {
        'heuristic': heuristic,
        'alpha': alpha,
        'beta': beta,
        'lam': lam,
        'k': k,
    }
    
    Sigma_h5 = sorted(set().union(*sequences))
    Sigma = Sigma_h5
    
    Prev = build_prev_table(sequences, Sigma)

    C_h5 = build_suffix_counts(sequences, Sigma_h5)
    seq_lens = [len(s) for s in sequences]
    if heuristic == "h8":
        sigma = len(set().union(*sequences))
        max_len = max(len(s) for s in sequences)
        score_args['sigma'] = sigma
        score_args['P_table'] = build_P_table(max_len, sigma)
    elif heuristic == "h5":
        score_args['C_h5'] = C_h5
        score_args['Sigma_h5'] = Sigma_h5
        score_args['seq_lens'] = seq_lens

    # --- globalni best u IMBS ---
    best_seq: str = ""
    best_steps: List[Tuple[int, ...]] = []

    # --- kandidati korijeni R: početak sa globalnim korijenom (-1,...,-1) ---
    global_root = tuple([-1]*m)
    R: List[Tuple[int, ...]] = [global_root]
    R_seen: Set[Tuple[int, ...]] = {global_root}

    # --- unutrašnja lokalna beam pretraga koja počinje iz skupa korijena izvora ---
    # -- cache za čuvanje rezultata backward BS-a za korijene --
    def run_local_beam_from_sources(sources: List[Tuple[int, ...]], cache: dict) -> Tuple[ str, List[Tuple[int, ...]], Optional[Tuple[int, ...]], List[Tuple[int, ...]  ], List[Tuple[int, ...]]  ]:
        """
        Vraća:
          (best_seq_local, best_steps_local, last_pos_of_best, harvested_new_roots)
        """
        #print("run_local_beam_from_sources")
        # Inicijalizacija bima sa zadanim izvorima
        beam: List[Node] =  [_mk_root_node_with_nonempty_seq(source, m, cache) for source in sources] #_mk_root_node
        best_local = ""
        return_node = None

        harvested_roots: Set[Tuple[int, ...]] = set()
        
        iters = 0

        while beam and iters < max_iters:
            iters += 1
            candidates: List[Node] = []

            for node in beam:
                # Proširi potomke
                succs = generate_successors(node, sequences, gaps)

                # Opcionalno skraćivanje potomaka na osnovu UB
                # (ne koristi se u eksperimentima) --- možda može i preko h5 UB?
                # best_len = len(best_seq)
                # pruned = []
                # for succ in succs:
                #     potential = succ.length + _ub_remaining(succ, sequences)
                #     if potential > best_len:
                #         pruned.append(succ)
                # succs = pruned

                # --- nema potomaka -> generiši r^w ---
                skip_root_check = False
                if not succs: # ako je kompletan cvor bez potomaaka -> generiši nove kandidate za korijene cvorove 
                    for rpos in _new_roots_from_node(node, sequences, gaps): #gen. nove korjene (zanemari gap ogranicenja)
                        
                        if rpos in R_seen:  
                            continue 
                        
                        #if not _feasible_root(rpos, sequences, len(best_seq)):
                        #    continue
                        
                        # izbaci one koji nisu "pravi" korijeni
                        if skip_root_check: #or not is_true_root_like(rpos, gaps, Prev, Sigma):
                            continue
                        harvested_roots.add(rpos)
                    continue

                # --- inače standardno proširujemo beam ---
                for succ in succs:
                    candidates.append(succ)

                    if len(succ.seq) > len(best_local):
                        best_local = succ.seq
                        return_node = succ #the best 

            if not candidates:
                break

            if heuristic == "h8":
                sigma = score_args['sigma']
                rems_per_v = [
                    min(len(sequences[i]) - v.pos[i] + 1 for i in range(m))
                    for v in candidates
                ]
                min_rem = min(rems_per_v)
                score_args['kh8'] = max(1, min_rem // sigma)

            # Rangiranje i skraćivanje bima
            beam = sorted(
                candidates,
                key=lambda n: score(n, sequences, gaps, **score_args),
                reverse=True  
            )[:beam_width]

        steps: List[Tuple[int, ...]] = []
        last_pos = return_node.pos if return_node is not None else None
        
        if return_node is not None:
            best_local = return_node.seq
            while return_node.parent is not None:
                if return_node.pos[0] == -1: #(-1, ..., -1)  global root -- we stop here
                    break
                steps.append(return_node.pos)
                return_node = return_node.parent
            steps.reverse()

        return best_local, steps, last_pos, list(harvested_roots), return_node.pos if return_node else []
        
        
    #===============================================================================
    ### Backward BS approach (bi-directional BS)
    #===============================================================================

    def generate_backward_successors(

        node: Node,
        sequences: List[str],
        gaps: List[List[int]],

    ) -> List[Node]:

        m = len(sequences)
        p =[px for px in node.pos] # p-vector of positions from where to start the backward pass
    
        successors: List[Node] = []

        for a in Sigma:
            prev_positions = []

            feasible = True
            for i in range(m):
                pi = p[i]
                if pi < 0:
                    feasible = False
                    break
                pa = Prev[i][a][pi]
                #print(i, ": ", pi, " ", pa, " ", gaps[i][pi - 1])
            
                if pa == -1:
                    feasible = False
                    break
                    
                if pi - pa - 1 > gaps[i][pi]:
                    feasible = False
                    break 
            
                prev_positions.append(pa)

            if not feasible:
                continue

            # build child
            new_pos = tuple(pa for pa in prev_positions)
            #print("buliding the child: ", new_pos)
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
        *,
        beam_width: int = 10,
        heuristic: str = "h5",
        max_iters: int = 10000
    ) -> Tuple[str, List[Tuple[int, ...]]]:
        """
        Backward Beam Search for Variable-Gapped LCS.
    
        Starts from position vector p and expands backward.
        """
        #print(f"Backward BS execution with {start_pos}")
        m = len(sequences)

        root = Node(start_pos, "", None)
        beam: List[Node] = [root]

        best_node: Optional[Node] = None
        best_len = 0

        score_args = {
            "heuristic": heuristic,
            "alpha": alpha,
            "beta": beta,
        }

        for _ in range(max_iters):
            
            if not beam:
                break  

            candidates: List[Node] = []

            for node in beam:
                succs = generate_backward_successors(
                    node, sequences, gaps
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
                key=lambda n: h5_backward(n, C_h5, Sigma_h5, seq_lens), #sort by the UB_2 criterion
                reverse=True,
            )[:beam_width]

        # ------------------------------------------------------------
        # Reconstruct backward solution
        # ------------------------------------------------------------

        steps: List[Tuple[int, ...]] = []
        seq = ""

        if best_node is not None:
            node = best_node
            seq = node.seq[::-1]  + sequences[0][start_pos[0]] # reverse backward-built sequence
            #print("best seq: ", seq)
            while node.parent is not None:
                if node.pos[0] == -1: #we come to the end 
                    break
                steps.append(node.pos)
                node = node.parent
            #steps.reverse() #increasing order of match occurances in the list of tuples

        return seq, steps #for the start_pos we return (seq, steps) pair ... 


    #==================================================================================================


    # --- IMBS outer loop ---
    cache = {} # save the nodes that are processed backward 
    time_start = time.time()

    for _ in range(imbs_iters):  

        #time exceeded, break condition 
        if time.time() - time_start > time_limit:
            break

        if not R:
            break

        # 1) izaberi korijen iz R (ranking acc. to UB2)
        nodes = [_mk_root_node(pos, m) for pos in R]
        ranked_nodes = sorted(
            nodes,
            key=lambda n: score(n, sequences, gaps, heuristic="h5", C_h5=C_h5, Sigma_h5=Sigma_h5, seq_lens=seq_lens),
            reverse=True
        )
        #take top-k roots
        R = [n.pos for n in ranked_nodes]
        take = R[:number_of_roots]
        R = R[number_of_roots:]
        
        #Take "roots" for evaluation with bacward BS
        for p_root in take:
            sol, coordinates = beam_search_glcs_backward(sequences, gaps, p_root)
            cache[ p_root ] = tuple([ sol, coordinates ] )
        
        # 2) BS iz ovog korijena (forward)
        seq_local, steps_local, last_pos_local, new_roots, the_root_with_best_path = run_local_beam_from_sources(take, cache)
        #new_roots should be evaluated in the next iterations  

        #print("seq_local:", seq_local, "\n len best: ", len(best_seq))

        # 3) ažuriranje globalnog besta ako je poboljšan
        length_refined = len(seq_local) #+ (0 if not the_root_with_best_path or the_root_with_best_path not in cache.keys() else len(cache[ the_root_with_best_path ][0]))
        if length_refined >= len(best_seq):
            
            best_seq = seq_local #cache[ the_root_with_best_path ][0] + sequences[0][the_root_with_best_path[0]] +  seq_local #length_refined
            best_steps = cache[ the_root_with_best_path ][1] + [the_root_with_best_path] + steps_local
            
                
            #print("Best steps: ", best_steps, " =============> ", cache[the_root_with_best_path], " root: ", the_root_with_best_path)
        
        # 4) ubacivanje novih korijena u R ako su feasible i nisu već viđeni
        for pos in new_roots:  

            if pos not in R_seen: #and _feasible_root(pos, sequences, len(best_seq)):
                R.append(pos)
                R_seen.add(pos)

    return best_seq, best_steps, time.time() - time_start       
    
    

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
    final_seq, final_steps, runtime = "", [], 0  # initialize to avoid UnboundLocalError
    

    import time
    start = time.time()

    if args.algorithm == "imbs":
        final_seq, final_steps, runtime = imbs_glcs(
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
            max_iters=args.max_iters, 
            time_limit=1800 # 30 minutes time limit
        )

    elif args.algorithm == "bs":
        final_seq, final_steps, runtime = beam_search_glcs(
            sequences,
            gaps,
            beam_width=args.beam_width,
            heuristic=args.heuristic,
            alpha=args.alpha,
            beta=args.beta,
            lam=args.lam,
            k=args.k,
            max_iters=args.max_iters, 
            time_limit=1800 # 30 minutes time limit
        )


    # Ensure final_steps is never None for feasibility check
    if final_steps is None:
        final_steps = []

    feasible = check_feasibility_n(final_steps, gaps, sequences, verbose=True)

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

