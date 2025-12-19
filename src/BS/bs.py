import heapq
from utils import *

class Node:
    def __init__(self, positions, length, parent=None):
        self.positions = positions  # tuple of positions in each sequence
        self.length = length        # current length of the common subsequence
        self.parent = parent        # optional, for backtracking or heuristics
    
    def __lt__(self, other):
        # Required for heapq; compare by heuristic value
        return heuristic(self) > heuristic(other)  # max-heap behavior

def beam_search(sequences, gap_funcs, beta):
    """
    Beam search for MLGCS problem.
    
    :param sequences: List of sequences s_1, ..., s_m
    :param gap_funcs: List of gap functions G_1, ..., G_m
    :param beta: Beam width (int)
    :return: Length of the best common subsequence found
    """
    m = len(sequences)
    initial_node = Node(positions=tuple([0]*m), length=0)
    beam = [initial_node]
    best = 0

    while beam:
        candidates = []
        
        for v in beam:
            new_nodes = extend(v, sequences, gap_funcs)
            candidates.extend(new_nodes)

        if candidates:
            best += 1
            # Sort candidates by heuristic and keep top beta
            top_candidates = heapq.nlargest(beta, candidates)
            beam = top_candidates
        else:
            break

    return best
