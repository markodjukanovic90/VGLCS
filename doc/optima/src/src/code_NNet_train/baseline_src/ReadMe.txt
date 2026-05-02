Compilation: In order to run the program, just write "make" and hit Enter


Parameters of the terminal call:

-i: path to an instance to be solved
-o: path to an file where the output information will be stored
-a: type of the algorithm to be executed {0: Greedy, 1: BS, 2: A*, 3: Approx1, 4: Approx2, 5: DP (for m=2, k=1), 6: DP (the most general version)}
-b: beam width 
-h: heuristic to be used {0: UB_1, 1: UB_2, 2: UB=min(UB1, UB2), 3: greedy_function, 4: Probabilistic-based guidance}
-t: timelimit  (in seconds) allowed for solving  
-pctext (optional), by default is 1/3: percentage of nodes from V_ext in beam search to be taken for constructing probability-based guidance

Example of a call (beam searc guided by UB with a beam with of 100):

./main -i ../instances/Rahman/converted/data_StrEC-converted/g15.txt -a 1 -o out-g15.txt -b 100 -h 2 -t 1000


./home1/djukanovic/lcps/src/srcAugust/lcps/src/rlcs/main -i $1 -o $2 -a $3 -b $4 -h $5 -t $6


#JAVA program 
     java -jar Automaton.jar ../Data/randomSeqEc/d32_0.txt out-automaton-d32_0.txt algorithm
      prvi argument: input file 
      drugi argument: out file 
      treci argument: algorithm (0: Automat -- 1: DP approaches of Farhana 2015)

Observe: read the instances from the directorium Data of this program...

