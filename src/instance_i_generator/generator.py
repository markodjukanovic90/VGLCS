import random

def generate_sequence(length, alphabet):
    """Generiše jednu sekvencu određene dužine."""
    return ''.join(random.choices(alphabet, k=length))

def generate_gap_function(length, min_gap, max_gap):
    """Generiše gap funkciju (listu celobrojnih vrednosti) za sekvencu."""
    return [random.randint(min_gap, max_gap) for _ in range(length)]

def generate_instance(num_sequences, seq_length, alphabet, filename):
    """Generiše celu instancu i čuva je u fajlu."""
    with open(filename, 'w') as f:
        f.write(f"{num_sequences}\n")
        for _ in range(num_sequences):
            seq = generate_sequence(seq_length, alphabet)
            gaps = generate_gap_function(seq_length, int(0.5*len(alphabet)), int(1.5*len(alphabet)))
            f.write(seq + "\n")
            f.write(' '.join(map(str, gaps)) + "\n")

    
def generate_group(num_of_instances_group=10, m=3,  n=50, sigma=['A', 'C', 'G', 'T']):
    
    #run: 
    for index in range(num_of_instances_group):
    	generate_instance(
        	num_sequences=m,
        	seq_length=n,
        	#min_gap=1,
        	#max_gap=3,
        	alphabet=sigma,
        	filename="mglcs_" + str(m) + "_" + str(n) + "_" + str(len(sigma)) + "_" + str(index) + ".txt"
	)
	
if __name__ == "__main__":
    # Parametri: broj nizova, dužina svakog niza, maksimalan gap, abeceda, ime izlaznog fajla
	nx=[500, 1000]  #50, 100, 200]
	mx=[2, 3, 5, 10]
	sigma=['A','T','G','C']
	
	for n in nx:
	    for m in mx:
	        for sx in sigma:  
	            generate_group(10, m, n, sigma)
