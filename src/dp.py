from utils import *
import time as tx
import sys 

## The basic DP za dvije niske (Peng and Yang, Section 2):
def compute_vgcs_vf(A, B, G_A, G_B):
    n = len(A)
    m = len(B)

    # Initialize V and F with 0s
    V = [[0 for _ in range(m + 1)] for _ in range(n + 1)]
    F = [[0 for _ in range(m + 1)] for _ in range(n + 1)]

    for i in range(1, n + 1):
        for j in range(1, m + 1):
        
            if A[i - 1] == B[j - 1]:
                # Compute F[i][j]
                max_f = 0
                for i_ in range(1, i):
                    if i - i_ <= G_A[i - 1] + 1 :
                        for j_ in range(1, j):
                            if j - j_ <= G_B[j - 1] + 1 and A[i_-1] == B[j_-1]:
                                max_f = max(max_f, F[i_][j_]) #matchings contribute here
                # update for + 1
                F[i][j] = max_f + 1

                # Compute V[i][j]
                V[i][j] = max(F[i][j], V[i - 1][j], V[i][j - 1])
            else:
                # Case A[i] != B[j]
                V[i][j] = max(V[i - 1][j], V[i][j - 1])
                F[i][j] = 0

    return V[n][m], V, F
    

##main ==> execute the basic DP for LGCSP
if __name__=="__main__":

    #terminal params to include:
    terminal_params=sys.argv[1 :]
    #print(terminal_params)
    file_in=terminal_params[0]
    file_out=terminal_params[1]

    s1, s2, G_s1, G_s2 = load_instance(file_in) #"mglcs_3_50_4_2.txt")
    
    begin_measure=tx.time()
    result, V, F = compute_vgcs_vf(s1, s2, G_s1, G_s2)
    end_measure=tx.time()
    #print(V)
    #solution reconstruction:
    sol, components = reconstruct_from_basic_vglcs_dp(s1, s2, G_s1, G_s2, V, F) #reconstruct_vglcs_sequence
    feasible=check_feasibility(components, s1, s2, G_s1, G_s2)
    #Stats:
    info_size=f"VGLCS length: {result}"
    info_time=f"Time: {round(end_measure-begin_measure, 2)}"
    sol_and_feasibility=f"Components: {components} \nFeasible: {feasible}"
    info_to_write=info_size+"\n"+info_time + "\n" + sol_and_feasibility
    
    print(info_size)
    print(info_time)
    print(sol)
    print(sol_and_feasibility)

    #solution = backtrack_lgcs(s1, s2, G_s1, G_s2) V-matrix serves here 
    #print(f"Solution is {solution}")
    #save into a file:
    save_in_file(file_out, info_to_write )
    
    
