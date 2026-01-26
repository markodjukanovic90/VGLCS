from numpy import *
from numpy import random as rd
import sys, getopt


### instance parameters
path_to_instance_dir = ""

#### instance characteristics #####

m=3
n=200
k=3 #number of patterns 
len_p=0.01 # lenght of patterns, a percentage of n 
sigma=4
size_group=5 #index of the insrance 

## function to generate a group of random instances
def generate(n, m, k, len_p, sigma, size_group):
    
    for index in range(size_group):
        f=open(path_to_instance_dir + str(m) + "_" + str(n) + "_" +  str(k) + "_"  + str(int(n * len_p)) + "_" + str(sigma) + "_" + str(index) + ".txt" , "w")
        
        # first line:
        f.write(str(m) + "\t" + str(sigma) + "\t" + "0" + "\t" + str(k) + "\n")
        
        ## fill in strings from S:
        for _ in range(m):
            s_i=''
            for _ in range(n):
                number = rd.randint(0, sigma-1)
                s_i = s_i + (chr(ord('A')+number))
                
            f.write(str(n) + "\t" +  s_i + "\n")
        
        
        ## generate strings from P:
        for i in range(k):
            p_i=""
            for _ in range(int(n * len_p)):
            
            	number = rd.randint(0, sigma-1)
            	p_i = p_i + (chr(ord('A')+number))
            
            if i < k-1: 
                f.write(str(int(n * len_p)) + "\t" +  p_i + "\n")
            else: 
                f.write(str(int(n * len_p)) + "\t" +  p_i )
            
        f.close()



if __name__ == "__main__":

   '''
   ##read from terminal line call:
   separator = ''
   argv = sys.argv[1: ]

   try:
      opts, args = getopt.getopt( argv,"h m:n:s:k:l:", ["help=", "m=", "n=", "s=", "k=", "l=" ] )
   except getopt.GetoptError:
      print('random_generator.py -m <br niski> -n <duzina niske> -s <duzina alfabeta> -k < br. instanci u grupi P> -l <duzina p_i stringova> >')
      sys.exit(2)
   #print opts
   
   for opt, arg in opts:
      print("args ==> " + arg)
      if opt == '-h':
         print('test.py -m <m> -n <n> -s <sigma> -k <broj niski u P> -l <duzina nisi u P>')
         sys.exit()
         
      elif opt in ("-m", "--m"):
         m = int(arg)
      elif opt in ("-n", "--n"):
         n = int(arg)
      elif opt in ("-s", "--s"):
         sigma = int(arg)
      elif opt in ("-k", "--k"):
         k = int(arg)
      elif opt in ("-l", "--l"):
         len_p = float(arg)     

   # print params extracted
   print('m ====> ', m)
   print('n ===> ', n)
   print('sigma ==> ', sigma)
   print('k ===> ', k)
   print('len_p ===> ', len_p)
   #main(sys.argv[1: ])
   '''    
   
   nx = [200, 500, 1000]
   mx = [3, 5, 10]
   kx = [3, 5, 10]
   lenpx = [0.01, 0.02, 0.05]
   #sigma = [4]
   #size_group=[5]   
   
   for n in nx:
      for m in mx:
          for k in kx:
              for lx in lenpx:
                  generate(n, m, k, lx, 20, 5) # sigma, size_group)  
        


