from utils import *
import time as tx
import sys 

def vglcs_optimized(A, B, G_A, G_B):
    n, m = len(A), len(B)

    # Inicijalizacija matrica
    F = [[0] * (m + 1) for _ in range(n + 1)]
    V = [[0] * (m + 1) for _ in range(n + 1)]
    Col = [[0] * (m + 1) for _ in range(n + 1)]
    All = [[0] * (m + 1) for _ in range(n + 1)]

    for i in range(1, n + 1):
        for j in range(1, m + 1):
            # Računanje All[i][j] = max iz pravougaonika F[i-1-GA : i-1][j-1-GB : j-1]
            row_start = max(1, i - G_A[i - 1] - 1)
            col_start = max(1, j - G_B[j - 1] - 1)
            
            max_val = 0
            for x in range(row_start, i):
                for y in range(col_start, j):
                    max_val = max(max_val, F[x][y])
            
            All[i][j] = max_val

            if A[i - 1] == B[j - 1]:
                F[i][j] = 1 + All[i][j]
                V[i][j] = max(F[i][j], V[i - 1][j], V[i][j - 1])
            else:
                F[i][j] = 0
                V[i][j] = max(V[i - 1][j], V[i][j - 1])

            # Opciono: možete popuniti Col[i][j] ako se koristi za drugu optimizaciju
            #Col[i][j] = max(F[i - 1][j], Col[i - 1][j]) if i > 1 else 0

    return V[n][m], V, F  # dužina najduže VGLCS


##main ==> execute advanced DP (Algorithm 2):

if __name__ == "__main__":

    #terminal params to include:
    terminal_params=sys.argv[1 :]
    #print(terminal_params)
    file_in=terminal_params[0]
    file_out=terminal_params[1]

    s1, s2, G_s1, G_s2 = load_instance(file_in)
    #run the algorithm:
    begin_measure=tx.time()
    result, V, F = vglcs_optimized(s1, s2, G_s1, G_s2)
    end_measure=tx.time()
    
    #stats:
    info_size=f"VGLCS length: {result}"
    info_time=f"Time: {round(end_measure-begin_measure, 2)}"
    #print(V)
    sol, comps = reconstruct_from_basic_vglcs_dp(s1, s2, G_s1, G_s2,  V, F) #reconstruct_vglcs_sequence
    feasible=check_feasibility(comps, s1, s2, G_s1, G_s2)
    sol_and_feasibility=f"Components: {comps} \nFeasible: {feasible}"
    # combine the info
    info_to_write=info_size+"\n"+info_time + "\n" + sol_and_feasibility
    print(info_size)
    print(info_time)
    print(sol_and_feasibility)
    #print(comps)
    assert len(comps) == len(sol), "|Comps| and |sol| not equal!!!"
    #save into a file:
    save_in_file(file_out, info_to_write)
    
    
    

