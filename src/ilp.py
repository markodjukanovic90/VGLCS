import pulp
from pulp import PULP_CBC_CMD, CPLEX_CMD
from utils import *
import sys
import time as tx
import orloge # pip install orloge
from os import remove
from math import ceil
from os import name
import random as rd

## GLOBAL VARS:
cplex_path=r'/home/marko/Desktop/cplex/cplex/bin/x86-64_linux/cplex'

def non_conflict(pair1, pair2, G_1, G_2):
    
    if(pair1[0] == pair2[0] or pair1[1] == pair2[1]):
        return False
    
    if ( pair1[0] >= pair2[0] and pair1[1] <= pair2[1] ) or  ( pair1[0] <= pair2[0] and pair1[1] >= pair2[1] ): #LCS-conflict
        return False 
    
    return True

def solve_vglcs_ilp_with_predecessors(A, B, G_A, G_B, time_limit_in_seconds=1200, components=[]):

    n, m = len(A), len(B)

    # Step 1: All valid match positions (i, j)
    M = [(i, j) for i in range(n) for j in range(m) if A[i] == B[j]]

    # Step 2: Define the ILP
    prob = pulp.LpProblem("VGLCS_with_Predecessors", pulp.LpMaximize)

    # Binary variables for selected matches and start points
    x = pulp.LpVariable.dicts("x", M, cat="Binary")
    s = pulp.LpVariable.dicts("s", M, cat="Binary")  # s_{i,j} = 1 if (i,j) starts a sequence

    # Step 3: Objective: maximize number of selected matches
    prob += pulp.lpSum(x[i, j] for (i, j) in M)

    # Step 4: Predecessor constraints of (i, j)
    for (i, j) in M:
        # Find valid predecessors of (i, j)
        preds = [
            (ip, jp) for (ip, jp) in M
            if ip < i and jp < j and (i - ip <= G_A[i]+1) and (j - jp <= G_B[j]+1)
        ]

        if preds:
            # (i,j) can only be active if one of its valid predecessors or it's a start
            prob += x[i, j] <= pulp.lpSum(x[ip, jp] for (ip, jp) in preds) + s[i, j], f"pred_link_{i}_{j}"
        else:
            # No predecessors — can only be selected if it's a start
            prob += x[i, j] <= s[i, j], f"no_pred_{i}_{j}"

    # Step 5: Only one start node is allowed
    prob += pulp.lpSum(s[i, j] for (i, j) in M) <= 1, "one_start"
    
    # Step 6: checking feasibility:
    if len(components) > 0: #add the fixed constraints:
        for (i, j) in components:
            prob += x[i, j] == 1
    
    
    #conflict constraints w.r.t. LCS conflict:
    for p1 in M:
        for p2 in M:
            if p1 != p2:
                if non_conflict(p1, p2, G_A, G_B)==False:
                   prob += ( x[ p1[0], p1[1] ]  + x[ p2[ 0 ] , p2[1] ] ) <= 1
    # Step 6: Solve&Collect
    random_number=rd.randint(0, 100000000)  
    log_file=f"log_info_{random_number}.log"
    
    prob.solve(CPLEX_CMD( path=cplex_path, timeLimit=time_limit_in_seconds, logPath=log_file, msg=False, threads=1))   #PULP_CBC_CMD(msg=1, timeLimit=time_limit_in_seconds))
    # Step 7: Extract the selected pairs
    selected_pairs = [ (i, j) for (i, j) in M if pulp.value(x[i, j]) >= 0.5 ]
    vglcs_length = len(selected_pairs)
    #read log file:
    logs_dict = orloge.get_info_solver(log_file, "CPLEX" ) # Orloge returns a dict with all logs info
    best_bound, best_solution, status = logs_dict["best_bound"], logs_dict["best_solution"], logs_dict["status"]
    #print("Best bound: ", best_bound) 
    #print(logs_dict)
    remove(log_file)
    #selected_pairs: check feasibility:
    feasible=check_feasibility(selected_pairs, s1, s2, G_s1, G_s2)
    selected_pairs.sort()
    
    return best_solution, selected_pairs,  best_bound, status, feasible
    #return vglcs_length, selected_pairs, status, feasible

    
#main:
if __name__ == "__main__":

    #terminal params to include:
    terminal_params=sys.argv[1 :]
    #print(terminal_params)
    file_in=terminal_params[0]
    file_out=terminal_params[1]
    time_limit=int(terminal_params[2])
    
    #check feasibility of this solution fixed_componentsfir
    fixed_components=[] 
    
    s1, s2, G_s1, G_s2 = load_instance(file_in)
    #print(G_s1, "\n", G_s2)
    begin_measure=tx.time()
    vglcs_length, selected_pairs, best_bound, status, feasible = solve_vglcs_ilp_with_predecessors(s1, s2, G_s1, G_s2, time_limit, fixed_components) #solve_vglcs_ilp
    end_measure=tx.time()  
    #print stats:  
    info_size=f"VGLCS length: {vglcs_length}"
    info_time=f"Time: {round(end_measure-begin_measure, 2)}"
    status=f"Status: {status}"
    sol_feas=f"Solution: {selected_pairs}\nFeasibility: {feasible}\nGap: {best_bound}"
    
    info_into_write=info_size + "\n" + info_time + "\n" + status + "\n" + sol_feas
    #print the info:
    print(info_size)
    print(info_time)  
    print(selected_pairs)
    #print(status)
    print(sol_feas)
    
    #save into a file:
    save_in_file(file_out, info_into_write)
