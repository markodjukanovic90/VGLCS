from collections import deque
from utils import *
import time as tx
import sys 

## runt the improved DP approach. It utilizes the sorted monothonic queues for each row 

def sliding_max(arr, start, end):
    """Return max over arr[start:end+1] using deque."""
    dq = deque()
    for i in range(start, end + 1):
        while dq and arr[dq[-1]] <= arr[i]:
            dq.pop()
        dq.append(i)
    return arr[dq[0]] if dq else 0

def vglcs_optimized(A, B, G_A, G_B):
    n, m = len(A), len(B)

    F = [[0] * (m + 1) for _ in range(n + 1)]
    V = [[0] * (m + 1) for _ in range(n + 1)]
    All = [[0] * (m + 1) for _ in range(n + 1)]

    for i in range(1, n + 1):
        ga = G_A[i - 1]
        row_start = max(0, i - ga - 1)
        row_end = i - 1

        for j in range(1, m + 1):
            gb = G_B[j - 1]
            col_start = max(0, j - gb - 1)
            col_end = j - 1

            # Compute max in rectangle [row_start:row_end][col_start:col_end]
            max_val = 0
            for x in range(row_start, row_end + 1):
                if col_start <= col_end:
                    row_max = sliding_max(F[x], col_start, col_end)
                    max_val = max(max_val, row_max)

            All[i][j] = max_val

            if A[i - 1] == B[j - 1]:
                F[i][j] = 1 + All[i][j]
                V[i][j] = max(F[i][j], V[i - 1][j], V[i][j - 1])
            else:
                F[i][j] = 0
                V[i][j] = max(V[i - 1][j], V[i][j - 1])

    return V[n][m], V, F
    
def vglcs_incremental_max(A, B, G_A, G_B):
    n, m = len(A), len(B)

    F = [[0] * (m + 1) for _ in range(n + 1)]
    V = [[0] * (m + 1) for _ in range(n + 1)]

    # Monotonic queues for each column (1-based indexing)
    column_max = [deque() for _ in range(m + 1)]

    for i in range(1, n + 1):
        g_a = G_A[i - 1]

        # Update column-wise monotonic queues based on previous row
        for j in range(1, m + 1):
            # Remove outdated rows from the front of the deque
            while column_max[j] and column_max[j][0][0] <= i - g_a - 1:
                column_max[j].popleft()

            # Insert F[i-1][j] into the deque, maintaining monotonicity
            if i > 1:
                current_val = F[i - 1][j]
                while column_max[j] and column_max[j][-1][1] <= current_val:
                    column_max[j].pop()
                column_max[j].append((i - 1, current_val))

        for j in range(1, m + 1):
            g_b = G_B[j - 1]

            # Compute the maximum value in the rectangle using column_max
            max_val = 0
            for y in range(max(1, j - g_b), j):
                if column_max[y]:
                    max_val = max(max_val, column_max[y][0][1])

            if A[i - 1] == B[j - 1]:
                F[i][j] = 1 + max_val
            else:
                F[i][j] = 0

            V[i][j] = max(F[i][j], V[i - 1][j], V[i][j - 1])

    return V[n][m], V, F
##main:

if __name__ == "__main__":

    #terminal params to include:
    terminal_params=sys.argv[1 :]
    #print(terminal_params)
    file_in=terminal_params[0]
    file_out=terminal_params[1]
    
    s1, s2, G_s1, G_s2 = load_instance(file_in)
    begin_measure=tx.time()
    result, V, F =   vglcs_optimized(s1, s2, G_s1, G_s2)
    end_measure=tx.time()
    sol, components = reconstruct_from_basic_vglcs_dp(s1, s2, G_s1, G_s2, V, F) #reconstruct_vglcs_sequence
    feasible=check_feasibility(components, s1, s2, G_s1, G_s2)
    #print(V)
    #print stats:
    info_size=f"VGLCS length: {result}"
    info_time=f"Time: {round(end_measure-begin_measure, 2)}"
    sol_and_feasibility=f"Components: {components} \nFeasible: {feasible}"
    # combine the info
    info_to_write=info_size+"\n"+info_time + "\n" + sol_and_feasibility
    print(info_size)
    print(info_time)
    print(sol_and_feasibility)
    #save into a file:
    save_in_file(file_out, info_to_write)
    
    





